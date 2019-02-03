# CubemapToFisheye
Working with Unity, I was in dire need of an accurate, and near real-time simulation of fisheye lense. Since a lot of computer vision projects use OpenCV to calibrate their cameras, I decided to use OpenCV's fisheye distortion model. Despite it's many flaws, this model is very popular so it seemed like a good choice to implement this effect accordingly. To simulate the fisheye effect in Unity efficiently as possible, first I build a look-up table (LUT) using a C++ code, and then I use it in a compute shader in Unity.

## OpenCV Fisheye Distortion Model
So as you probably know, in a fisheye lense straight lines appear curved. Just like getting drunk, where there are many ways in which you go from a functioning human being to a pitiful, drooling mess curled on the floor, it works similarily with straight lines and fisheyes. The model in which OpenCV transforms lines is an equidistant fisheye projection, with perturbations. 

![Equidistant model](https://www.researchgate.net/publication/299374422/figure/fig5/AS:349739804577799@1460395875601/Equidistant-projection-equidistant-projection-th-90dc-R-35.png)

Instead of using the ideal model

                        r = f * θ 
                        
OpenCV suggests a more realistic approach by model the angel as a series:

                        θd=θ * (1 + k1 * θ2 + k2 * θ4 + k3 * θ6 + k4 * θ8)
                        r = f * θd
                        
Where (k1, k2, k3, k4) are the fisheye's distortion coefficients. 

## The Magic of Cubemaps


## Building LUTs in C++


## Integrating the CubemapToFisheye effect in Unity


## Planned Features
- Chromatic aberration
- Optimizations

## License
MIT, I think. But if you buy me a 6-pack of diet coke I won't complain.
