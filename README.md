# CubemapToFisheye
Working with Unity, I was in dire need of an accurate, and near real-time simulation of fisheye . Since a lot of computer vision projects use OpenCV to calibrate their cameras, I decided to use OpenCV's fisheye distortion model. Despite its many flaws, this model is very popular so it seemed like a good choice to implement this effect accordingly. To simulate the fisheye effect as efficiently as possible, first I build a look-up table (LUT) using a C++ code, and then I use it in a compute shader in Unity.

[![](http://img.youtube.com/vi/3-XyOb4pdns/0.jpg)](http://www.youtube.com/watch?v=3-XyOb4pdns "FisheyeSimulatorDemo")

## OpenCV Fisheye Distortion Model
As you probably know, in a fisheye lens straight lines appear curved. Just like getting drunk, where there are many ways in which you go from a functioning human being to a pitiful, drooling mess curled on the floor, it works similarily with straight lines and fisheyes. The model in which OpenCV transforms lines is an equidistant fisheye projection, with perturbations. 

![Equidistant model](https://www.researchgate.net/publication/299374422/figure/fig5/AS:349739804577799@1460395875601/Equidistant-projection-equidistant-projection-th-90dc-R-35.png)

Instead of using the ideal model

                        r = f * θ 
                        
OpenCV suggests a more realistic approach by modelling the angel as a series:

                        θd = θ * (1 + k1 * θ2 + k2 * θ4 + k3 * θ6 + k4 * θ8)
                        r = f * θd
                        
Where (k1, k2, k3, k4) are the fisheye's distortion coefficients. 

## The Magic of Cubemaps
Cubemaps are like social media. You think there is a whole world around you, but it's just six stupid faces structured as a box with you in the center, and you're too dumb to realize it's a facade. It's basically a very efficient way of environmental mapping and it's excessively used in computer graphics. A cubemap is generated by rendering 6 views from the same point - the view directions are on 3 orthogonal axes (e.g. +x, +y, +z, -x, -y, -z), and each view has a frustum of 90°. 

![Cubemap](http://i.imgur.com/32X3hcc.png)

Unity offers a class that renders cubemaps for you, and it's probably more efficient than the one I wrote, but it's limited to cubemaps that are aligned with the Cartesian axes. So if you want to model a tilted camera, you're fucked. 

## Building LUTs in C++
Currently, I don't have a CMake. So you can use my .sln/.vcxproj files in VS2017, or take the source files and create a new project in whatever IDE. Just make sure you link it with OpenCV. 

In orer to create a LUT, you will need to provide LUT builder with an intrisic matrix (k) in a CV_64F format (double), a vector of 4 doubles for the distortionn coeeficients (cv::Vec4d), the resolution of the lens that matches these calibration paramaters and the resolution of the cubemap faces. Cubemap face resolution must be a square and a power of 2.
LUTs can be saved to a folder as binary files, and also be read from binary files. 

## Integrating the CubemapToFisheye effect in Unity
I'm going to upload a demo application in Unity so you can see how to use my awesome shit. I'm not sure how to upload the app without all the unnecessary Unity junk files yet. 
Basically, there are two important files that you can tweak to suit your needs. CubemapToFisheye.cs that you attach as a script to your unity camera model, and the compute shader, CubemapToFisheye.compute.

### Operating Modes
Currently, there are two operating modes, capturing and calibration. Both automatically generate a fisheye every x frames, but capturing mode buffers all the generate fisheyes to be saved to file when the application is closed, while calibration mode will buffer only the few latest fisheyes, and will save to file only when you press "enter".

You can find in the source code a calibration scene, with controllable checkerboard, that allows you to take calibration images of the simulated fisheye created with the LUT. When the board is positioned to you liking, press "enter" and the image will be saved to the destination folder. Once you have taken a few dozen images, you can run the OpenCV calibration methods for fisheye lense and verify your model. I might upload an example code soon if I'm not too lazy.

[![](http://img.youtube.com/vi/rOP5NLxpJyI/0.jpg)](http://www.youtube.com/watch?v=rOP5NLxpJyI "CalibrationApp for FisheyeSimulatorDemo")


### CubemapToFisheye script
The script renders cubemaps, generates fisheye images and saves them to a destination folder when you close the program (so I/O won't slow the application while running), or saves selected images, depending on the mode. You must add another dummy camera to actually see where you're going - I'm rendering the original camera to texture, and then sending the simulated fisheye images to another camera.  
You can easily tweak it so it won't save the images to files, or the frequency at which the fisheyes are generated. 

### CubemapToFisheye Compute Shader
If you are not familiar with compute shaders, I wouldn't touch this one.

### Using the GUI
When I create the cubemap, I query about the camera's location and orientation, so just configure the transform component to your liking and it will take effect automatically. 
In the CubemapToFisheye script, you need to fill in the desired cubemap resolution (remember, a power of 2!), the destination folder to save the generated fisheye images and the folder where the pre-build LUT is stored. You can also choose your operating mode.

## Planned Features
- Chromatic aberration
- Optimizations
- CMake

## License
MIT, I think. But if you buy me a 6-pack of diet coke I won't complain.
