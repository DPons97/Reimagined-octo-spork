# [PII] ROS - Reimagined Octo-Spork
#### Heterogeneous distributed system for object tracking and fun stuff

This project involves object recognition with Darknet and OpenCV's YOLO algorithm running on a distributed system for object tracking.

Requirements:
---

**Server-side**:
* @pjreddie's **[Darknet YOLO](https://pjreddie.com/darknet/yolo/)** (Already included in this repo) - https://github.com/pjreddie/darknet/
  + ***.cfg** that are in this repository have been tweaked to be faster with less powerful NVIDIA GPUs*
  + ***yolov3.weights** + **yolov3-tiny.weights** files are required and need to be placed inside **Server/darknet/***

**Client-side:**
* **OpenCV 3.4.5** - https://github.com/opencv/opencv/releases/tag/3.4.5 
  + **OpenCV_DNN** additional modules - https://github.com/opencv/opencv_contrib/releases/tag/3.4.5
  + Installation guide here: https://docs.opencv.org/3.2.0/de/d25/tutorial_dnn_build.html

* A *LOT* of patience

Installation:
---
After cloning this repository (```git clone https://github.com/DPons97/reimagined-octo-spork.git ```):
* Build darknet inside ```Server/darknet/``` (and test it!)
* Build OctoSpork: 
  ```bash
  cd [...]/reimagined-octo-spork/
  mkdir build
  cmake ..
  make [leave empty if you want to build everything. Otherwise, see below]  
  ```
  If you want to only build Server, change ```make``` line to:
  ```shell
  make OctoSporkServer
  ```
  Otherwise, to build Client only:
  ```shell
  make OctoSporkClient
  ```
  
* If you decided to build Client, you have to move and compile *bkgSubtraction* and *nodeTracker* inside ***Client/Executables***.

Running ROS:
---
cd into the build directory in reimagined-octo-spork
```bash
cd path-to/reimagined-octo-spork/build
```
* Server side:
  ```bash
  ./OctoSporkServer
  ```
* Client side:
  There are two ways to run the client executable:
  * Passing all informations as arguments
  ```bash
  ./OctoSporkClient [ipaddress] [port] [node_id] [node_x] [node_y] [theta] [top_neighbour] [bottom_n] [left_n] [right_n] 
  ```
  * Passing a configuration file
  ```bash
  ./OctoSporkClient [path_to_cfg_file]
  ```
  The configuration file should be a one line file with the arguments you would pass as above.
  
  **NB:** *node_x*, *node_y*, *theta* are respectively the coordinates and the phase displacement of the camera's node. 


Tested Hardware:
---
* Odroid xu4
* Jetson-TX2 with Nvidia Tegra technology
* Any PC with any Linux distribution (Tested on Fedora and Ubuntu)
<br><br>

The idea:
---
The initial goal was to develop a software that involved distributed heterogeneous systems and that would be flexible and reusable for different applications.<br>
As we needed to work on a realistic case study, we thought about diving into image recognition.<br>
First, we did some research to find the algorithms we had at disposal and to select which one was the most suitable for our hardware.<br>
Pjreddie's darknet network is really good concerning detection and it's fast enough on the Jetson but, having to also use an Odroid XU4 (8 cores CPU, no GPU), YOLO's library would result in really high computational times.<br>
Here's where OpenCV's Deep Neural Network module comes in handy: it features a 9x faster implementation of DNN using CPU with the same darknet's yolov3 configuration file.<br>
Some tweaking to the cfg are required to achive an acceptable speed, although trading off some accuracy.<br>
Now let's **talk about the fun stuff.**<br>

How does it work?
---
The ROS system's architecture is based on one SERVER (in our case the Jetson-TX2) and one or more CLIENTS / NODES (Odroid XU4 or any linux pc).<br>
Every client connects to the server sending his planimetry informations (his node's ID and his neighbours' IDs) and waits for new instructions to be executed.<br>
Client nodes relay on a text file to match the instruction ID received from the server with an actual executable that it can run in a new process. In this way the client side is easy to customize (more on that later... ).
The server communicates every node an "idle" operation that is, in our case, a *Background subtraction* process. <br><br>
**NB**: If you are not into detection and tracking systems, there still is something for you. Just skip the next paragraphs and go to *Customizing ROS.* <br><br>

![alt text](https://github.com/DPons97/reimagined-octo-spork/blob/master/Concept.jpg)

**NB**: As a lot of people don't have multiple cameras at their disposal, we implemented the video stream as a series of images (https://trac.ffmpeg.org/wiki/Create%20a%20thumbnail%20image%20every%20X%20seconds%20of%20the%20video). <br>
Just remember to change FPS and other parameters inside bkgSubtraction and nodeTracker.<br>

### Background subtraction
During this initial phase every client applies background subtraction (provided by OpenCV) to a given video stream.<br>
Once a blob that is big enough is detected, the node sends last frame that was analyzed to the server to run a first object detection in search of certain user-defined objects.<br>
If something is found (inside our project we search for people), a new *Tracking* instruction is sent to the node that found the blob.<br>

### Tracking
If a client receives this instruction, it starts tracking the defined object that should be in his sight.<br>
For every frame that has the object in it, the node saves detected box's coordinates and tries to estimate distance from the camera (with bad results, for now :neutral_face:).<br>
When the object is no more in sight of the client's camera, the server receives all saved coordinates and analyze them to decide whether it could keep tracking through other connected cameras.<br>
This is possible thanks to the planimetry that is stored inside server.<br>

<br><br>
#### Project Contributors:
* Luca Collini [@Lucaz97](https://github.com/Lucaz97) - luca.collini@mail.polimi.it
* Davide Pons [@DPons97](https://github.com/DPons97) - davide.pons@mail.polimi.it
<br>
