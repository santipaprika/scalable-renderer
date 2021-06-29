# Scalable Renderer

## Description
Real-time renderer built on C++ with scalability features and optimization algorithms for graphics and game engines. It involves _LOD_ generation, time-critical rendering, and visibility preprocessing, among others.

Final project for the _UPC_ course _Scalable Rendering for Graphics and Game Engines_.

## Demo
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/btikFe5sELE/0.jpg)](https://youtu.be/btikFe5sELE)

## Dependencies

The following external libraries have been used (all included in the
source folder):

-   Glut

-   GLM

-   Eigen 3.3.9

-   ImGui

Also, for museum tuning purposes the application [Tiled](https://www.mapeditor.org/) has been used.

## How to compile

In the project directory (where all dependencies should be already
included) create a *build* folder and run the *cmake* command from it:

``` bash
mkdir build
cd build
cmake ..
```

Then, compile using the generated *Makefile* in the same *build*
directory.

``` bash
make -j
```

Finally, run the program by executing the generated binary *BaseCode*:

``` bash
./BaseCode
```

## How to use

### Initial debug information

: If the application seems to be frozen, it is probably performing some
computations. The application outputs debug messages in the terminal
often so that the user can do a follow-up of which actions are being
done.

### Setting up the museum

The museum is defined as a *.tmx* file which can be generated through
the [Tiled](https://www.mapeditor.org/) application.  
The current file being used can be found in the folder *museum* inside
the source directory. Since the program is parsing the file at this
location, it is important to avoid reallocating it. The actual museum
file is called *tilemap.tmx*, and once it is opened in *Tiled*, one can
modify its content easily.  
The color map that will appear by default in the editor is shown in
the following figure with each value to element correspondence:
<img src="https://i.ibb.co/RB3VrtJ/colormap.png" alt="colormap" width="30%">
| Empty  |    Floor    |  Origin   |   Cube    |
|:------:|:-----------:|:---------:|:---------:|
| Bunny  |   Dragon    |   Frog    |   Happy   |
| Horse  |    Lucy     | Maxplanck |   Moai    |
| Sphere | Tetrahedron |   Torus   | Armadillo |

### Setting up the models

All models being used in the current scene are provided in the *models*
folder.  
The *LOD* generation phase can take two paths:

-   If the *LOD* does not exist in disk: Generate from scratch all the
    *LOD*s using the selected technique.

-   If the *LOD* exists in disk: Read and assign it like an usual mesh.

*LOD* files are stored in the same *models* directory, with the
following nomenclature:

<div class="center">

NAME_REP_CLUST_LOD.ply

</div>

Where *NAME* stands for the original name of the model, *REP* is the
representation technique, which can be average (*AVG*) or quadric error
metrics (*QEM*), and *CLUST* is the clustering technique, which can be
standard voxel clustering (*VOX*) or voxel clustering with normal
clustering (*VOX-NC*). *LOD* indicates the level of the *LOD*
(6,7,8...).  
All *LOD*s being used in the current scene are included as well in the
*models* folder, so that loading times are minimized.  
If one wants to generate them from scratch, the *LOD* *.ply* files
should be removed (not the original detailed ones!). If this is the
case, I suggest removing only the *LOD*s of a specific object.
Otherwise, loading times can be unnecessarily large (1-5 mins in the
current system), depending on the combination of techniques being used.
In the current scene, this process is highly slowed by the *dragon.ply*
model, since it is much more complex and takes more time to process.

### Movement

Standard *FPS* controls (*WASD*) with *E* (up) *Q* (down). Also, while
the space bar is pressed, the camera speed increases.

### Visibility

Visibility works similar to the *LOD* generation phase. Initially,
visibility is computed and stored into a file *visibility.vis* in the
same build folder, which is afterwards parsed in the main application.
However, in the current implementation visibility computation is
included in the application, so it will detect automatically whether
there is already a visibility file to use or it should generate a new
one. The application does not detect changes in the museum, so if any
change in the floor shape is done, the visiblity file should be manually
removed.  
The visibility computation step can be slightly tuned by the user by
modifying the number of rays to use per cell. This can be done in the
*VisiblityComputer.cpp* file by changing the *N_RAYS* preprocessor macro
value. To generate an accurate visibility file I suggest using 20 rays
per cell, which takes less than a minute in the current system. Still, I
provide a visibility file obtained from using 100 rays per cell, which
should be even more accurate. **In case one wants to use it, the file
needs to be moved manually to the build folder**.

### Using the GUI

When running the application, a GUI viewport will appear (on top left by
default) on the window. **By default, the application will take control
of the mouse pointer**. To recover it, do a left click and the pointer
should appear as usual. This is useful to change options from the GUI.  
If some options are not fully visible, its size can be extended by
dragging the corners.  
The available GUI options and their effects on the application are:  

-   **Framerate**

-   **Render points**: See points instead of triangles.

-   **Render wireframe**: See wireframe triangles instead of filled
    triangles.

-   **Representative computation strategy**: *LOD* representation
    technique to use per node:

    -   Average (*AVG*)

    -   QEM

    Changing this option will reload the scene with the mesh *LOD*s
    using the selected technique. Notice that if these *LOD*s do not
    exist in disk, they will be generated from scratch, which may take a
    significant amount of time depending on the model.

-   **Vertex clustering strategy**: *LOD* clustering technique to use
    per node:

    -   Voxel clustering (*VOX*)

    -   Voxel with normal clustering (*VOX + NC*)  
    Changing this option may take some additional time because of the same reason stated in the previous point.
    
-   **Show colored *LOD*s**: from lower to higher *LOD*: Red, orange,
    yellow, green.

-   **Use fixed *LOD*s**:

    -   If enabled, time-critical rendering will be disabled and all
        models will have the same *LOD*, modifiable by the user.

    -   If disabled, time-critical rendering will be enabled and the
        triangles per second (*TPS*) measure will be displayed as a
        slider so that the user can manipulate to find appropiate
        values. Higher *TPS* values will select higher *LOD*s in general
        and viceversa.

-   **Use hysteresis:**

    -   If disabled, use standard time-critical rendering (notice
        popping effects)

    -   If enabled, two modes will be available:

        -   Absolute distance: Individual *LOD* transitions will be
            re-allowed after moving closer/further to the object than
            the defined **fixed distance** in the slider below (with
            respect to the distance at the blocking moment between the
            camera and the object).

        -   Relative distance: Individual *LOD* transitions will be
            re-allowed after moving closer/further to the object than
            the d**distance from the node to the the camera scaled by
            the defined factor** below.

        In my opinion, absolute distance is more consistent since
        relative distance cannot avoid popping if the object is very
        close at the moment of blocking.

-   **Use visibility**: If not checked, all entities will be rendered
    and taken into account for time-critical rendering.

-   **Freeze visibility**: To see which nodes are being hidden at a
    given point, this button can be pressed and visibility will be
    frozen to that point while the camera can navigate through the
    museum. A similar test can be done simply by going up (*E*) and
    observing that entities from other rooms are not being rendered
    (since visibility is computed in 2D grid space).
