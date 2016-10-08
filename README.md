# MY3D

This project is a software 3D renderer with basic functions according to rendering pipeline whose coordinate system is same as DX3D system.

For studying purpose,the project just has only one C++ file.Two render states are included,WIREFRAME and INTERPOLATION COLOR.The rendering process is as follow:

1.Spilting the polygon into triangles and sending three vertices into rendering pipeline.

2.World transform,view transform , projection transform(perspective)and transform homogenizing.

3.Since perspective is used,backface culling is processed after transform homogenizing.

4.drawing the triangle by scan conversation and interpolating color while updating z-buffer value(INTERPOLATION COLOR)/
  draw the wire by DDA(WIREFRAME).
