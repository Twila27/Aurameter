Assignment Name: Job System
Assignment Number: 07 (Final)
Name: Benjamin Gibson
Class: Software Development V
Instructor: Chris Forseth
	

//-----------------------------------------------------------------------------	
A7 Implemented Systems
	- All but Job Thread suspending (requires semaphores/signaling) and Job Dependency Chains. Find implementations for both in lecture notes.
	- Profiling notes at the bottom of this ReadMe.
	
A7 Known Issues
	- Object Pool isn't thread-safe! Needs a critical section, but right now that causes a crash shortly after start ("called abort()").
	- Job system doesn't seem to produce a speed increase all that impressively when profiled.
	
A6 Known Issues
	- Smoke seems to be intermittently drawn at full-size, possibly the frame of a particle's creation.
	- Particle systems, especially ones created with ParticleSystem::Play(), end up briefly rendering the previously bound sprite texture for an unknown reason.
	
A5 Known Issues
	- Profiler prints to logger (and hence also VS debugger output pane) instead of on-screen because for most of A5 my DFS graphics problems kept me from being able to do 2D full-screen rendering.
	- Memory tracking in the profiler introduces a significant framerate drop. (Event registration calls are built out of non-profiling builds for this reason.)
	
A3 Known Issues
	1. Sprite rotation doesn't work on the carrot ship sprite as would be expected from TheGame::Update.
		- Tried to change rotation to be around x (the forward direction in engine basis) instead of z axis, the sprite simply didn't rotate at all then.
	2. Perhaps because of the rotation, the visibility culling (off by default) is too agressive when rotating and scrolling the camera.
	3. Couldn't nail down the binding of FBOs correctly for layer effects to work. 
		- Only basic all-layers FBO effects from SD3 work (ToggleFBOs, then SetFBOsCurrentEffectIndex commands).
		- However, Obama and Twirl FBO effects broken.
	4. No time to get to the sprite sheet cutter.
	5. Layer ordering has gotten reversed, so in order to render UI on top of the game, UI had to be given -1000 instead of +1000 for a layer ID.
	6. Instead of providing a per-layer world bounds, because I wanted scrolling tied to the Camera2D class, my setup only supports a flag to ignore scrolling altogether.
		- The Camera2D itself is given an AABB2 to limit how far it can translate around.


//-----------------------------------------------------------------------------
Build Notes
	.exe will crash if it cannot find these directories from its working directory $(SolutionDir)Run_$(PlatformName)/ under Project Properties:
	Data/Fonts
	Data/Models
	Data/Images
	Data/Shaders
	Data/XML
	
	
//-----------------------------------------------------------------------------
A5 Console Commands
	ProfilerChangeReportView (list view default, toggle from flat view)
	ProfilerPause
	ProfilerPrintLastFrame (to logger / vs debugger, debug.log in .exe directory by default)
	ProfilerTogglePaused
	ProfilerUnpause
	
	
	
//-----------------------------------------------------------------------------
A3 Console Commands (tab to autocomplete or cycle, nothing new for A4)
	SpriteRendererCreateOrRenameLayer
	SpriteRendererDisableLayer
	SpriteRendererEnableLayer
	SpriteRendererListLayers / SpriteRendererPrintLayers
	SpriteRendererLoadAllSprites -- by default, the project currently loads all files matching *.Sprites.xml in Data/XML/SpriteResources directory
	SpriteRendererSaveAllSprites
	SpriteRendererToggleCulling -- recommended to hit F1 to see a report of # sprites culled in top-right
	SpriteRendererToggleLayer
	SpriteRendererToggleSpriteParenting
	ToggleShowing3D -- disables a ClearScreenToColor call that masks over the 3D scene behind the SpriteRenderer's rendered frame
	ToggleActiveCameraMode -- defaults to 2D controls, but combined with SpriteRendererToggleShowing3D lets you access the 3D flycam


//-----------------------------------------------------------------------------
General Controls
	WASD: Move
	P: Play/Pause
	R: Return to Menu (when paused or at game over)
	Enter: Gain Terrain's Deflector (during gameplay)
	Spacebar: Launch Deflector (during gameplay)
	ESC: Exit
	F1: Toggle Debug Info
	
Camera2D Controls
	Locked for this game.
	
Camera3D Controls
	Removed for this game.

Console Mode Controls
	Tilde: Cycle Showing Console (between unshown, shown read-only, and shown with input prompt states)
	Tab: Auto-complete (use with blank input to cycle through all commands)
	Enter: Execute/Close
	Escape: Clear/Close
	PageUp/PageDown/Scroll: Scroll Output (necessary for 'help' command, some are pushed off-screen now)
	Left/Right: Move Caret
	Home/End: Move Caret
	Delete: Delete
	Up/Down: Command History
	
	
//-----------------------------------------------------------------------------
Resource Attributions
	Fonts		BMFont Software
	Input Font	http://input.fontbureau.com/
	Images		Chris Forseth, OpenGameArt and Ansimuz (CC0 Public Domain Collections @ opengameart.com, ansimuz.com/site/archives/category/resources)
	Models		Chris Forseth, Unity Technologies Japan, Komal K.

//-----------------------------------------------------------------------------

Before Job System

//title
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 228290  (0.089963ms)	 1651    (0.000651ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 66388   (0.026162ms)	 66388   (0.026162ms)	 29.08  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 160251  (0.063151ms)	 160251  (0.063151ms)	 70.20  %		 0       (0    b)	 0       (0    b)
	
//enemies/particles
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 293256  (0.115564ms)	 1813    (0.000714ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 55810   (0.021993ms)	 55810   (0.021993ms)	 19.03  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 235633  (0.092857ms)	 235633  (0.092857ms)	 80.35  %		 0       (0    b)	 0       (0    b)	

//enemies/particles
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 309449  (0.121946ms)	 1524    (0.000601ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 56125   (0.022117ms)	 56125   (0.022117ms)	 18.14  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 251800  (0.099228ms)	 251800  (0.099228ms)	 81.37  %		 0       (0    b)	 0       (0    b)	

After Job System

//title
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 153839  (0.060624ms)	 523     (0.000206ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 46402   (0.018286ms)	 46402   (0.018286ms)	 30.16  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 106914  (0.042132ms)	 106914  (0.042132ms)	 69.50  %		 0       (0    b)	 0       (0    b)
	
//first transition
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 196684  (0.077508ms)	 482     (0.000190ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 46299   (0.018245ms)	 46299   (0.018245ms)	 23.54  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 149903  (0.059073ms)	 149903  (0.059073ms)	 76.22  %		 0       (0    b)	 0       (0    b)	

//enemies/particles
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 230672  (0.090902ms)	 716     (0.000282ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 46080   (0.018159ms)	 46080   (0.018159ms)	 19.98  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 183876  (0.072461ms)	 183876  (0.072461ms)	 79.71  %		 0       (0    b)	 0       (0    b)	

//enemies/particles + had a very full console open
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 330787  (0.130355ms)	 504     (0.000199ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 72751   (0.028669ms)	 72751   (0.028669ms)	 21.99  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 257532  (0.101487ms)	 257532  (0.101487ms)	 77.85  %		 0       (0    b)	 0       (0    b)	

//after fixing the bug, still seems slow
[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 340029  (0.133997ms)	 923     (0.000364ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 82687   (0.032585ms)	 82687   (0.032585ms)	 24.32  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 256419  (0.101048ms)	 256419  (0.101048ms)	 75.41  %		 0       (0    b)	 0       (0    b)	

[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 327257  (0.128963ms)	 1024    (0.000404ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 82368   (0.032459ms)	 82368   (0.032459ms)	 25.17  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 243865  (0.096101ms)	 243865  (0.096101ms)	 74.52  %		 0       (0    b)	 0       (0    b)	

[Profiler] DEPTH	 TAG                           	 #CALLS	 TIME (PerfCount, ms) 	 SELF-TIME            	 %FRAME-TIME	 #ALLOCS (#Bytes) 	 #FREES  (#Bytes) 	
[Profiler] 0    	 Main                          	 1     	 391528  (0.154291ms)	 647     (0.000255ms)	 100.00 %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Update             	 1     	 82725   (0.032600ms)	 82725   (0.032600ms)	 21.13  %		 0       (0    b)	 0       (0    b)	
[Profiler] 1    	 TheEngine::Render             	 1     	 308156  (0.121436ms)	 308156  (0.121436ms)	 78.71  %		 0       (0    b)	 0       (0    b)	

//end sd5 a7 profiling.