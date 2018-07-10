# Real-Time Volumetric Cloud Renderer
<img src="res/readme/logo.gif">

This project introduces an efficient and effective method to render animated, lifelike clouds in real-time. Our clouds dynamically react to other world-parameters like the position of the camera or the sun. Our clouds are also conveniently parameterized to allow for flexible customization of its density, position, sizes, etc.

## Results
<img src="res/readme/r1.gif" width=50%><img src="res/readme/r2.gif" width=50%>

## Method and Features
* Interactive real-time smoke rendering
* Spherical voxelization of a 3D texture using billboards
* Voxel cone tracing

## Development Details
_[Devlog](http://www.jaafersheriff.com/search/label/clouds)_

_[Final implementation paper](paper/senior-project.pdf)_

## Libraries Used
* [GLFW](http://www.glfw.org/)
* [GLM](https://glm.g-truc.net/0.9.8/index.html)
* [glad](https://github.com/Dav1dde/glad)
* [ImGui](https://github.com/ocornut/imgui)
* [stb_image.h](https://github.com/nothings/stb)

## References
* Sam Freed. "Tessellated Voxelization for Global Illumination using Voxel Cone Tracing." Master's Thesis 2018. California Polytechnic State University, San Luis Obispo Digital Commons. 
* Robert Larsson. "Interactive Real-Time Smoke Rendering." Master's Thesis 2010. Chalmers University of Technology. 
* Cyril Crassin, Fabrice Neyret, Miguel Sainz, Simon Green, Elmar Eisemann. Interactive Indirect Illumination Using Voxel Cone Tracing. NVIDIA Research 2011. 
