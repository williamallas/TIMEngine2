# TIMEngine2
Second version of my Graphics Engine

Written in C++ / OpenGL4

Features :
- VR compatible (openvr)
- Tiled deferred shading for mass lights instancing
- Cascaded shadow mapping
- Physically based materials
- Global illumination approximation (using specular probes)
- Minimal driver overhead using glMultiDrawElementsIndirect/bindless textures/single allocated vertex buffer
- Screen space reflexion, FXAA, Bloom/HDR shaders
- A pipelined rendering system for flexible rendering, and pooled buffers
- A resource system for async loading and automatique unused resources deletion

Organisation : 
- Source/TIMEngine2 : 
Actual implementation of the engine, divided in modules : 
	- bullet : interface of the bullet physic engine
	- core : tools and maths for general purpose
	- renderer : interface for OpenGL and low level rendering stuffs
	- resource : the resource/asset system
	- scene : base class for scene management, non related to the rendering
	- interface : use everything above to implement a rendering pipeline and high level resource management
	
- Source/OpenVR : interface for openvr steam library for making the engine compatible with VR headsets.

- Source/TIMEditor :
An GUI editor for the engine, still in developement and used to design the scenes for my VR game.
Written with QT.

- Source/PortalGame : 
An unnamed VR puzzle game with physically based interactions and a novative way of discovering the world using portals.

- Source/RTSEngine : the (very) beginning of a RTS game engine
