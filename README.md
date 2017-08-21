# Installation
- Add the PLUGIN to your project
- Place a 'Character Controller' to the scene
 - Change 'Auto Posses Player' to Player 0
 - Check the following components to the 'Character Controller' and set them up as you need: MovementComponent, OpenCloseComponent,  PickupComponent,  LogComponent
- Copy the 'Highlights'-folder into your content folder
- Select 'PostProcessVolume' in the world outliner, go to 'Rendering Features' and add the 'PP_Outliner_M.uasset' to 'Post Process Materials'
- For picking up items you have to specify empty 3 actors (should be attached to the character) as the hand positions

# Use the following tags (compatible with UTags plugin)
- `ClickInteraction;Interactable,True;OpenCloseable,True;` - For drawers and doors
- `ClickInteraction;Interactable,True;Pickup,True;`- For items that can be picked up

# USemLog 
- Add tags to use the USemLog Plugin (https://github.com/robcog-iai/USemLog/)

# Stack Stability Checker
- Add a Stack Checker Actor to the scene
- Set Collision Preset to `BlockAllDynamic`
- Add Animation Curves to `Rotation Animation Curve Vector` and `Shaking Animation Curve Vector`
- Set `Max Deviation` to determine how much a stacked item needs to change it's position to declare the stack unstable
- Check `Show Stability Check Window` if you want to see the stack being checked (See Stability Checker Scene Capture)
- Set `DelayBeforeStabilityCheck` to delay the stability check after shaking and rotating the stack

## Stability Checker Scene Capture
- To use a visual response while the stability check is running, add a ```Scene Capture 2D```Actor to the scene
- Bring the Scene Capture Camera into position (Orthographic is recommended)
- Set 'Texture Target' of the Scene Caprute 2D to RenderTargetTexture which is provided in ClickInteractions/Textures
- Also use the CIHUD resp. BP_CIHUD as HUD

# Controls

 - `Mouse` Look around
 - `W,S,A,D` Move around
 - `X` (hold) Crouch
 - `Left Mouse` Interact with objects. Pickup/Drop items (left hand)
 - `Right Mouse` Pick/Drop items (right hand)
 - `Left Control` Force Drop (When focusing on an interactable object but you want do drop your items onto it e.g. a drawer)
 - `Left Shift` Enable Drag-Mode
 - `TAB` Force pickup of single item

# Testing scenarios
Each scenario should be played in all three modes:

1. One Hand Mode
2. Two Hand Mode (Stacking disabled)
3. Two Hand Mode (Stacking enabled)

## Set up the scenario as followed
- Go to `CharacterController` and chose your scenario type you want to record (One, Two or Four Person Breakfast)
- Go to `CharacterController` and chose your interaction mode (One hand, two hands or two hand + stacking)

## Playing the scenario
- First, you should just fiddle around to get used to the controls
- After you figured everything out, read the scenario you want to play and restart the game
- After you played your scenarios, you can find the log events in the folder `SemLog`

# Scenarios
For each of the following scenarios there is an example image in the folder `ScenarioImages`

## One Person breakfast
Arrange the following items on the table:

- One small bowl
- One cup
- One glass
- One medium sized plate
- One knife
- One table spoon
- One pack of cereals
- One pack of milk
- One pack of juice
- One pack of crispbread or rusk (zwieback)

![OnePersonBreakfast](/ScenarioImages/OnePerson.png)


## Two person breakfast
Arrange the following items on the table:

- Two medium sized bowls
- Two cups
- Two table spoons
- One pack of cereals 
- One pack of milk


![TwoPersonBreakfast](/ScenarioImages/TwoPerson.png)


## Four person breakfast
Arrange the following items on the table:

- 4 small bowls
- 4 cups
- 4 glasses
- 4 medium sized plates
- 4 knifes
- 4 table spoons
- Two packs of crispbread or rusk (zwieback)
- Two packs of cereals
- Two packs of milk
- Two packs of juice


![FourPersonBreakfast](/ScenarioImages/FourPerson.png)
