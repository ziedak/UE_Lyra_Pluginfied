# Gas

CMD
D:/"Program Files"/"Epic Games"/UE_5.4/Engine/Build/BatchFiles/Build.bat Gas Win64 Development -project="E:/Unreal
Projects/ue5/zied/Gas/Gas.uproject"

MSBuildEnableWorkloadResolver: true

## Input System:

Resources:

- [Input System](https://docs.unrealengine.com/en-US/InteractiveExperiences/Input/index.html)
- [Input System Overview](https://docs.unrealengine.com/en-US/InteractiveExperiences/Input/InputSystemOverview/index.html)
- [Input Action System](https://docs.unrealengine.com/en-US/InteractiveExperiences/Input/InputActionSystem/index.html)
- [Input Mapping Context](https://docs.unrealengine.com/en-US/InteractiveExperiences/Input/InputMappingContext/index.html)
- [Input Routing](https://www.exportgeometry.com/blog/input-routing-deep-dive)
- [Introduction to Common UI](https://www.youtube.com/watch?v=TTB5y-03SnE&t=4581s)

### Enhanced Input System:

- This system pushes input bindings into data assets. There are two pieces:
- The input mapping context, which is essentially a replacement for the key binding map.
  They map from key->input_action
- The input actions, which define the actual events that you branch from.

### Common UI Input Action System

The common UI input action system is a half-baked collection of systems that let you do a couple different things:
Bind user input to widget functionality
Display icons for input keys
That’s basically it
CommonUI IAS consists of the following:

#### Input Action Table

This contains bindings from [action name]->[device specific keybinding]. This is a one-to-many relationship. So you
could bind the “jump” action to [spacebar] on a keyboard and [A] on an xbox controller.

#### InputControllerData

This basically just allows you to give each key on an input device (keyboard, controller, etc) an icon.

#### CommonUIInputData

Here you specify a universal “click” and “back” action. The action must link to a row in your input action table.

#### FNavigationConfig black magic:

You can actually rebind the engine definitions for basic actions like a “click”.

# Strings

Name = constant string
Sting = normal string
Text = Localizable text