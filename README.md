# Note: UTags is not implemented and used yet. Use the following tags:
- "Interactable" to highlight interactable objects
- "OpenCloseable" to add open and close functionality
- "RotateButton" to interact with rotating buttons
- "Pickup" to pickup and drop items

# Installation
- Add the PLUGIN to your project
- Place a 'Character Controller' to the scene
 - Change 'Auto Posses Player' to Player 0
 - Add the following components to the 'Character Controller':
  - CMovement 
  - COpenClose
  - CRotateButton
  - CPickup etc. Look what you need
- Copy the 'Highlights'-folder into your content folder
- Select 'PostProcessVolume' in the world outliner, go to 'Rendering Features' and add the 'PP_Outliner_M.uasset' to 'Post Process Materials'
- For picking up items you have to specify empty 3 actors (should be attached to the character) as the hand positions

# Stability Checker Scene Capture
- To use a visual response while the stability check is running, add a ```Scene Capture 2D```Actor to the scene
- Bring the Scene Capture Camera into position (Orthographic is recommended)
- Set 'Texture Target' of the Scene Caprute 2D to RenderTargetTexture which is provided in ClickInteractions/Textures
- Also use the CIHUD resp. BP_CIHUD as HUD

# Controls

 - `Mouse` Look around
 - `W,S,A,D` Move around
 - `Left Mouse` Interact with objects. Pickup/Drop items (left hand)
 - `Right Mouse` Pick/Drop items (right hand)
 - `Left Controls` Force Drop (When focusing on an interactable object but you want do drop your items onto it e.g. a drawer)