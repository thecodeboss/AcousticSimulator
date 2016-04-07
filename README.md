# Acoustic Simulator

This project simulates sound propagation in a 3D scene, based on the techniques described in the paper:

    Efficient and Accurate Sound Propagation using Adaptive Rectangular Decomposition
    Nikunj Raghuvanshi, Rahul Narain and Ming C. Lin
    IEEE Transactions on Visualization and Computer Graphics(TVCG), 15(5), 2009.

For PML boundary conditions, the formulation from the paper:

    Marcus G., S., Imbo.
    Efficient PML for the wave equation.
    arXiv:1001.0319, 2010.

was used, though we developed our own discretizations to the equations as the ones in the paper caused numerical issues for us.

An example of what the simulation accomplishes can be seen on YouTube by clicking the image below:

[![YouTube video of sound simulation](https://img.youtube.com/vi/FU0mSVFfBe0/0.jpg)](https://www.youtube.com/watch?v=FU0mSVFfBe0)

# Building and Running

To build, Visual Studio 2015 or later is required. The solution itself is self-contained, so simply building and running in Visual Studio should work.
