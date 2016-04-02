#include "PMLPartition.h"
#include <memory>

PMLPartition::PMLPartition(PMLType t, int gx, int gy, int w, int h, int d)
	: Partition(gx, gy, w, h, d), type(t) {
	int size = width*height + 1; // +1 so we can have a zero at the end
	p_old = (double*)malloc(size*sizeof(double));
	p = (double*)malloc(size*sizeof(double));
	p_new = (double*)malloc(size*sizeof(double));

	phi_x = (double*)malloc(size*sizeof(double));
	phi_x_new = (double*)malloc(size*sizeof(double));

	phi_y = (double*)malloc(size*sizeof(double));
	phi_y_new = (double*)malloc(size*sizeof(double));

	force = (double*)malloc(size*sizeof(double));

	// Zero out everything
	memset((void*)p_old, 0, size*sizeof(double));
	memset((void*)p, 0, size*sizeof(double));
	memset((void*)p_new, 0, size*sizeof(double));
	memset((void*)phi_x, 0, size*sizeof(double));
	memset((void*)phi_x_new, 0, size*sizeof(double));
	memset((void*)phi_y, 0, size*sizeof(double));
	memset((void*)phi_y_new, 0, size*sizeof(double));
	memset((void*)force, 0, size*sizeof(double));

	// Special interface handling
	includeSelfTerms = false;
	// Don't render PML regions
	shouldRender = false;

	switch (type) {
	case P_LEFT:
		kxMin = 0.2;
		kxMax = 0.0;
		break;
	case P_RIGHT:
		kxMin = 0.0;
		kxMax = 0.2;
		break;
	case P_TOP:
		kyMin = 0.2;
		kyMax = 0.0;
		break;
	case P_BOTTOM:
		kyMin = 0.0;
		kyMax = 0.2;
		break;
	default:
		break;
	}
}

PMLPartition::~PMLPartition() {
	free(p_old);
	free(p);
	free(p_new);
	free(phi_x);
	free(phi_x_new);
	free(phi_y);
	free(phi_y_new);
	free(force);
}

int PMLPartition::getIdx(int x, int y) {
	if (x < 0 || x >= width) return width*height;
	if (y < 0 || y >= height) return width*height;
	return y*width + x;
}

void PMLPartition::step(double t) {
	double dx = 1.0;
	double dy = 1.0;
	for (int i = 0; i < width; i++) {
		double kx = 0.0;
		double ky = 0.0;
		switch (type) {
		case P_LEFT:
			kx = (i < 20) ? (20 - i)*kxMin/10.0 : 0.0;
			ky = (i < 20) ? 0.05 : 0.0;
			break;
		case P_RIGHT:
			kx = (i > 20) ? (i - 20)*kxMax/10.0 : 0.0;
			ky = (i > 20) ? 0.05 : 0.0;
			break;
		default:
			break;
		}
		//double kx = (kxMax*i + kxMin*(width - 1 - i)) / width;
		for (int j = 0; j < height; j++) {
			switch (type) {
			case P_TOP:
				ky = (j < 20) ? (20 - j)*kyMin / 10.0 : 0.0;
				kx = (j < 20) ? 0.05 : 0.0;
				break;
			case P_BOTTOM:
				ky = (j > 20) ? (j - 20)*kyMax / 10.0 : 0.0;
				kx = (j > 20) ? 0.05 : 0.0;
				break;
			default:
				break;
			}
			//double ky = (kyMax*j + kyMin*(height - 1 - j)) / height;
			double coefs[] = { 2.0, -27.0, 270.0, -490.0, 270.0, -27.0, 2.0 };
			double KPx = 0.0;
			double KPy = 0.0;
			for (int k = 0; k < 7; k++) {
				KPx += coefs[k] * p[getIdx(i + k - 3, j)];
				KPy += coefs[k] * p[getIdx(i, j + k - 3)];
			}
			KPx /= 180.0;
			KPy /= 180.0;

			double c = speedOfSound;
			double term1 = 2 * p[getIdx(i, j)];
			double term2 = -p_old[getIdx(i, j)];
			double term3 = c*c*(KPx + KPy + force[getIdx(i, j)]);
			double term4 = -(kx + ky)*(p[getIdx(i, j)] - p_old[getIdx(i, j)])/dt;
			double term5 = -kx*ky*p[getIdx(i, j)];
			double dphidx = 0.0;
			double dphidy = 0.0;
			double fourthCoefs[] = { 1.0, -8.0, 0.0, 8.0, -1.0 };
			for (int k = 0; k < 5; k++) {
				dphidx += fourthCoefs[k] * phi_x[getIdx(i + k - 2, j)];
				dphidy += fourthCoefs[k] * phi_y[getIdx(i, j + k - 2)];
			}
			dphidx /= 12.0;
			dphidy /= 12.0;

			double term6 = dphidx + dphidy;
			p_new[getIdx(i, j)] = term1 + term2 + dt*dt*(term3 + term4 + term5 + term6);

			double dudx = 0.0;
			double dudy = 0.0;
			for (int k = 0; k < 5; k++) {
				dudx += fourthCoefs[k] * p_new[getIdx(i + k - 2, j)];
				dudy += fourthCoefs[k] * p_new[getIdx(i, j + k - 2)];
			}
			dudx /= 12.0;
			dudy /= 12.0;

			phi_x_new[getIdx(i, j)] = phi_x[getIdx(i, j)] - dt*kx*phi_x[getIdx(i, j)] + dt*speedOfSound*speedOfSound*(ky - kx)*dudx;
			phi_y_new[getIdx(i, j)] = phi_y[getIdx(i, j)] - dt*ky*phi_y[getIdx(i, j)] + dt*speedOfSound*speedOfSound*(kx - ky)*dudy;
		}
	}

	std::swap(phi_x_new, phi_x);
	std::swap(phi_y_new, phi_y);

	double* temp = p_old;
	p_old = p;
	p = p_new;
	p_new = temp;

	memset((void*)force, 0, width*height*sizeof(double));
}

double* PMLPartition::getPressureField() {
	return p;
}

double PMLPartition::getPressure(int x, int y) {
	return p[getIdx(x, y)];
}

void PMLPartition::setForce(int x, int y, double f) {
	force[getIdx(x, y)] = f;
}
