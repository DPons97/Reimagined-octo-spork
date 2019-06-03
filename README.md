# [PII]  Reimagined-Octo-Spork
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

Tested Hardware:
---
* Odroid xu4
* Jetson-TX2 with Nvidia Tegra technology
* Any PC with any Linux distribution (Tested on Fedora and Ubuntu)
<br><br>

The idea:
---
The initial goal was to try to develop a software that involved distributed heterogeneous systems.
As we needed to work on a realistic study case, we thought about diving into image recognition.
First, we did some research to find the algorithms we had at disposal and to select which one was the most suitable for our hardware.
Pjreddie's darknet network is really good concerning detection but, having to use an Odroid XU4 (8 cores CPU, no GPU), with YOLO's library it would result in really high computational times.
Here's where OpenCV's Deep Neural Network module comes in handy: it features a 9x faster implementation of DNN using CPU.


<br><br>
##### Project Contributors:
* Luca Collini [@Lucaz97](https://github.com/Lucaz97) - luca.collini@mail.polimi.it
* Davide Pons [@DPons97](https://github.com/DPons97) - davide.pons@mail.polimi.it
<br>
