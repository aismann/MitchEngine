![Mitch Engine Banner](https://raw.githubusercontent.com/wobbier/MitchEngine/master/Docs/GitHub/Havana.png)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/858846f643cc47258ed72f9cfddb28b2)](https://www.codacy.com/app/rastaninja77/MitchEngine?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=wobbier/MitchEngine&amp;utm_campaign=Badge_Grade)
![license](https://img.shields.io/github/license/wobbier/mitchengine.svg)
======
The 3D game engine so good it has my name in it.

Mitch Engine is a simple c++ component based game engine for creating games for Xbox One, macOS, and the Windows 10 Store.
It's a great hobby project to keep me exploring the world of c++.

Check out my [Trello Board][4] to check out the current development status.

|Platform|Game|
|---|---|
|UWP / Xbox|[![Build status UWP](https://ci.appveyor.com/api/projects/status/7x55po7se0siesdn?svg=true)](https://ci.appveyor.com/project/wobbier/mitchengine)|
|macOS|[![Build Status](https://travis-ci.com/wobbier/stack.svg?branch=macOS)](https://travis-ci.com/wobbier/stack)|

The engine is:

  * Open source
  * Easy to use
  * Awesome

Main features
-------------
   * (ECS) Entity-Component System based design
   * World Editor (Windows & macOS)
   * HTML UI with [Ultralight][5]
   * [BGFX][9] Rendering
   * [Optick][6] Profiling
   * Language: C++
   * Supports Windows, Xbox, and macOS

How to make a Mitch game
-----------------------

1. Download the [Project Setup][8] batch file and run that from your git repo root.
2. Think of an awesome game idea.
3. ????
4. Profit

Examples
-----------------------
```cpp
// Grab the world
SharedPtr<World> GameWorld = GetEngine().GetWorld().lock();

// Create an entity.
EntityHandle MainCamera = GameWorld->CreateEntity();

// Add some components
Transform& CameraTransform = MainCamera->AddComponent<Transform>("Main Camera");
Camera& CameraComponent = MainCamera->AddComponent<Camera>();

// Start changing some values
CameraTransform.SetPosition(Vector3(0.f, 5.f, 10.f));

// Spawning models.
EntityHandle ModelEntity = GameWorld->CreateEntity();

ModelEntity->AddComponent<Transform>("Ground Model");
ModelEntity->AddComponent<Model>("Assets/Models/SuperAwesome.fbx");
```

Build Requirements
------------------

* Windows 10
* Visual Studio 2017
	* Desktop Development with C++
	* Universal Windows Platform development
	* Game development with C++
	* C++ Universal Windows Platform tools
* CMake - 3.12.0 (Required if you wish to update ThirdParty projects)

Third Party Libraries
--------------------------------

  * [Ultralight][5]
  * [Optick][6]
  * [Bullet Physics][7]

Contributing to the Project
--------------------------------

Did you find a bug? Have a feature request?

  * [Contribute to the engine][2]

Contact me
----------

   * Website: [http://wobbier.com][1]
   * Twitter: [http://www.twitter.com/wobbier][3]

[1]: http://www.wobbier.com "My Portfolio"
[2]: https://github.com/wobbier/MitchEngine/issues "GitHub Issues"
[3]: http://www.twitter.com/wobbier "Twitter"
[4]: https://trello.com/b/QpR06bQl/mitchengine-status "Trello Board"
[5]: https://github.com/ultralight-ux/Ultralight "Ultralight"
[6]: https://github.com/bombomby/optick "Optick"
[7]: https://github.com/bulletphysics/bullet3 "Bullet 3D"
[8]: https://raw.githubusercontent.com/wobbier/MitchEngine/master/Tools/NewProjectSetup.bat "Project Creator"
[9]: https://github.com/bkaradzic/bgfx "BGFX"
