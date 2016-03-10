%%
% This code discretizes the 2D wave equation with PML boundary conditions.
% Written by Michael Oliver at the University of Waterloo, though the
% discretizations are based on the paper:
%
%    "Efficient PML for the wave equation"
%    by Marcus J. Grote and Imbo Sim
%    Jan 2, 2010
%
% In particular, it adds PML to the following 2D wave equation:
%
%   (d/dt^2) u = c^2*Lu + f
%
% where Lu is the Laplacian of u.

Sx = 100;
Sy = 100;
dt = 0.5;
c = 1;
dx = 1;
dy = 1;
k_max = 1;
steps = 250;
width = 10;

uo = zeros(Sx, Sy);
u = zeros(Sx, Sy);
un = zeros(Sx, Sy);

phi1 = zeros(Sx, Sy);
phi1n = zeros(Sx, Sy);

phi2 = zeros(Sx, Sy);
phi2n = zeros(Sx, Sy);

f = zeros(Sx, Sy);

for t = 1:steps
    f(50,50) = 2*sin((t-1)*pi/20);
    for i = 2:Sx-1
        if i <= width
            k1 = k_max*(width-i)^2/width^2;
        elseif i >= Sx-width
            k1 = k_max*(width-Sx+i)^2/width^2;
        else
            k1 = 0;
        end
        for j = 2:Sy-1
            if j <= width
                k2 = k_max*(width-j)^2/width^2;
            elseif j >= Sy-width
                k2 = k_max*(width-Sy+j)^2/width^2;
            else
                k2 = 0;
            end
            fac1 = (k1 + k2)*dt/2;
            fac2 = 2*u(i,j) - (1 - fac1)*uo(i,j);
            du2dx2 = (u(i+1,j)-2*u(i,j)+u(i-1,j))/(dx*dx);
            du2dy2 = (u(i,j+1)-2*u(i,j)+u(i,j-1))/(dy*dy);
            D2u = du2dx2 + du2dy2;
            dphi1dx = (phi1(i,j)+phi1(i,j-1)-phi1(i-1,j)-phi1(i-1,j-1))/(2*dx);
            dphi2dy = (phi2(i,j)+phi2(i-1,j)-phi2(i,j-1)-phi2(i-1,j-1))/(2*dy);
            Dphi = dphi1dx + dphi2dy;
            fac3 = c*c*D2u + Dphi;
            fac4 = k1*k2*u(i,j);
            un(i,j) = (fac2 + dt*dt*(fac3 - fac4))/(1+fac1) + f(i,j);

            fac5 = 1/dt + k1/2;
            fac6 = 1/dt - k1/2;
            dudx1 = (un(i+1,j)+un(i+1,j+1)-un(i,j)-un(i,j+1))/(2*dx);
            dudx2 = (u(i+1,j)+u(i+1,j+1)-u(i,j)-u(i,j+1))/(2*dx);
            dudx = (dudx1 + dudx2)/2;
            fac7 = fac6*phi1(i,j) + c*c*(k2-k1)*dudx;
            phi1n(i,j) = fac7/fac5;

            fac8 = 1/dt + k2/2;
            fac9 = 1/dt - k2/2;
            dudy1 = (un(i,j+1)+un(i+1,j+1)-un(i,j)-un(i+1,j))/(2*dy);
            dudy2 = (u(i,j+1)+u(i+1,j+1)-u(i,j)-u(i+1,j))/(2*dy);
            dudx = (dudy1 + dudy2)/2;
            fac10 = fac9*phi2(i,j) + c*c*(k1-k2)*dudx;
            phi2n(i,j) = fac10/fac8;
        end
    end
    phi1 = phi1n;
    phi2 = phi2n;
    uo = u;
    u = un;
    g = u;
    g(width,width:Sy-width) = 10;
    g(Sx-width,width:Sy-width) = 10;
    g(width:Sx-width,width) = 10;
    g(width:Sx-width,Sy-width) = 10;
    imshow((g + 1)/2,'InitialMagnification','fit'), colormap(jet(64)), colorbar;
    pause(0.01);
end
