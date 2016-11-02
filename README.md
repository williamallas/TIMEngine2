# TIMEngine2
Second version of my Graphics Engine

Written in C++ / openGL4

Features :
- VR compatible (openvr)
- tiled deferred shading for mass lights instancing
- Cascaded shadow mapping
- physically based materials
- Global illumination approximation (globaly and localy using specular prob)
- Minimal driver overhead using glMultiDrawElementsIndirect/bindless texture/single allocated vertex buffer
- Screen space reflexion, FXAA, Bloom/HDR shaders
- A pipelined rendering system for flexible rendering, and pooled buffers
- a resource system for async loading and automatique unused resource deletion

Organisation : 
- Source/TIMEngine2 : 
Actual implementation of the engine, divided in modules : 
	- bullet : interface of the bullet physic engine
	- core : tools and maths for general purpose
	- renderer : interface for openGL and all low level rendering stuffs
	- resource : the resource/asset system
	- scene : base class for scene management, non related to the rendering
	- interface : use everything above to implement a rendering pipeline and high level resource management
	
- Source/OpenVR : interface for openvr steam librairy for making the engine compatible with VR headsets.

- Source/TIMEditor :
An GUI editor for the engine, still in developpement and used to make every scene for my VR game.
Written mostly with QT.

- Source/PortalGame : 
An unnamed VR puzzle game with physically based interaction and a novative way of discovering the world using portals.

- Source/RTSEngine : the (very) beginning of a RTS game engine
