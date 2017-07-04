# Installation
- Add the PLUGIN to your project
- Place a 'Character Controller' to the scene
 - Change 'Auto Posses Player' to Player 0
 - Add the following components to the 'Character Controller':
  - CMovement 
  - COpenClose
  - CRotateButton
  - ...
- Copy the 'Highlights'-folder into your content folder
- Select 'PostProcessVolume' in the world outliner, go to 'Rendering Features' and add the 'PP_Outliner_M.uasset' to 'Post Process Materials'

# Controls

 - `Mouse` Look around
 - `W,S,A,D` Move around
 - `Left Mouse` Interact with objects. Pickup/Drop items (left hand)
 - `Right Mouse` Pick/Drop items (right hand)
 - `Left Controls` Force Drop (When focusing on an interactable object but you want do drop your items onto it e.g. a drawer)