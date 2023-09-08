# LibRetro Notes

This document is a compilation of my notes as I'm implementing LibRetro core support for RetroSim. 

Mainly I'll be examining how TIC-80 implements the LibRetro client.

## Implemented Functions

- `retro_init`: This function initializes the LibRetro core. It sets up necessary resources and configurations for the TIC-80 core to run within the LibRetro framework.
- `retro_deinit`: This function deinitializes the LibRetro core. It cleans up any resources that were allocated during the initialization process.
- `retro_api_version`: This function returns the version of the LibRetro API that the TIC-80 core supports.
- `retro_get_system_info`: This function provides information about the TIC-80 core, such as its name, version, and supported extensions.
- `retro_get_system_av_info`: This function provides audio and video information for the TIC-80 core, such as the base width, base height, and frames per second.
- `retro_set_environment`: This function sets up the environment callback for the LibRetro core. It allows the core to communicate with the frontend and request certain functionalities.
- `retro_set_video_refresh`: This function sets the video refresh callback, which is used to update the display.
- `retro_set_audio_sample`: This function sets the audio sample callback, which is used to play audio samples.
- `retro_set_audio_sample_batch`: This function sets the audio sample batch callback, which is used to play a batch of audio samples.
- `retro_set_input_poll`: This function sets the input poll callback, which is used to poll input devices.
- `retro_set_input_state`: This function sets the input state callback, which is used to get the state of input devices.
- `retro_set_controller_port_device`: This function sets the controller port device callback, which is used to handle controller input.
- `retro_reset`: This function resets the TIC-80 core to its initial state.
- `retro_run`: This function runs a single frame of the TIC-80 core. It handles input, updates the game state, and renders the frame.
- `retro_serialize_size`: This function returns the size of the serialized state. It's used for save states.
- `retro_serialize`: This function serializes the current state of the TIC-80 core into a buffer.
- `retro_unserialize`: This function loads a serialized state from a buffer into the TIC-80 core.
- `retro_cheat_reset`: This function resets cheats.
- `retro_cheat_set`: This function sets a specific cheat.
- `retro_load_game`: This function loads a game into the TIC-80 core.
- `retro_load_game_special`: This function loads a special game type.
- `retro_unload_game`: This function unloads the currently loaded game.
- `retro_get_region`: This function returns the region of the game.
- `retro_get_memory_data`: This function returns a pointer to the specified memory type.
- `retro_get_memory_size`: This function returns the size of the specified memory type.

These functions allow the TIC-80 core to interface with the LibRetro frontend, handle input, render graphics, play audio, and manage game states.

## Call Order

1. Initialization:
    - `retro_set_environment`: This is usually the first callback to be called. It sets up the environment callback for the core to communicate with the frontend.
    - `retro_init`: Initializes the core.

1. Configuration:
    - `retro_set_video_refresh`: Sets the video refresh callback.
    - `retro_set_audio_sample` and retro_set_audio_sample_batch: Set the audio callbacks.
    - `retro_set_input_poll` and retro_set_input_state: Set the input callbacks.
    - `retro_set_controller_port_device`: Sets the controller input callback, if needed.

1. Game Loading:
    - `retro_get_system_info`: Provides information about the core.
    - `retro_load_game` or retro_load_game_special: Loads a game into the core.

1. Main Loop:
    - `retro_run`: This is called repeatedly in a loop to run the core, handle input, update the game state, and render frames.

1. Save States (if supported and invoked by the user or frontend):
    - `retro_serialize_size`: Determines the size of the serialized state.
    - `retro_serialize`: Serializes the current state.
    - `retro_unserialize`: Loads a serialized state.

1. Cheats (if supported and invoked by the user or frontend):
    - `retro_cheat_reset`: Resets cheats.
    - `retro_cheat_set`: Sets a specific cheat.

1. Game Unloading:
    - `retro_unload_game`: Unloads the game from the core.

1. Deinitialization:
    - `retro_deinit`: Deinitializes the core.

1. Memory Access (if supported and invoked by the user or frontend):
    - `retro_get_memory_data`: Accesses specific memory data.
    - `retro_get_memory_size`: Gets the size of specific memory data.

1. Region Information (if needed):
    - `retro_get_region`: Returns the region of the game.

## Function Details

### **`retro_set_environment`**:  
   * **Functionality**: Sets up the environment callback for the core to communicate with the frontend. It allows the core to request certain functionalities or resources from the frontend.
   * **Arguments**: A function pointer to the environment callback.
   * **Description**: The core can send environment commands to the frontend, and the frontend can respond with the requested data or status.

### **`retro_init`**:
   * **Functionality**: Initializes the core. It sets up necessary resources and configurations for the core to run within the LibRetro framework.
   * **Arguments**: None.
   * **Description**: Initialization parameters and configurations are set up internally within the core.

### **`retro_deinit`**:
   * **Functionality**: Deinitializes the core. It cleans up any resources that were allocated during the initialization process.
   * **Arguments**: None.
   * **Description**: Cleanup parameters and configurations are handled internally within the core.

### **`retro_api_version`**:
   * **Functionality**: Returns the version of the LibRetro API that the core supports.
   * **Arguments**: None.
   * **Description**: The core communicates its supported API version to the frontend.

### **`retro_get_system_info`**:
   * **Functionality**: Provides information about the core.
   * **Arguments**: A pointer to a `retro_system_info` structure.
   * **Description**: The core fills the `retro_system_info` structure with details like its name, version, and supported extensions, which is then communicated to the frontend.

### **`retro_get_system_av_info`**:
   * **Functionality**: Provides audio and video information for the core.
   * **Arguments**: A pointer to a `retro_system_av_info` structure.
   * **Description**: The core fills the `retro_system_av_info` structure with AV details like base width, base height, and frames per second, which is then communicated to the frontend.

### **`retro_set_video_refresh`**:
   * **Functionality**: Sets the video refresh callback.
   * **Arguments**: A function pointer to the video refresh callback.
   * **Description**: The core uses this callback to send video frames to the frontend for rendering.

### **`retro_set_audio_sample`** and **`retro_set_audio_sample_batch`**:
   * **Functionality**: Set the audio callbacks.
   * **Arguments**: Function pointers to the audio sample and audio sample batch callbacks.
   * **Description**: The core uses these callbacks to send audio samples or batches of audio samples to the frontend for playback.

### **`retro_set_input_poll`** and **`retro_set_input_state`**:
   * **Functionality**: Set the input callbacks.
   * **Arguments**: Function pointers to the input poll and input state callbacks.
   * **Description**: The core uses the input poll callback to poll input devices and the input state callback to get the state of input devices.

### **`retro_set_controller_port_device`**:
   * **Functionality**: Sets the controller input callback.
   * **Arguments**: The port index and a device ID.
   * **Description**: The core communicates with the frontend about which devices are connected to which ports.

### **`retro_reset`**:
   * **Functionality**: Resets the core to its initial state.
   * **Arguments**: None.
   * **Description**: Reset parameters and configurations are handled internally within the core.

### **`retro_run`**:
   * **Functionality**: Runs a single frame of the core.
   * **Arguments**: None.
   * **Description**: The core handles input, updates the game state, and renders the frame, communicating any necessary data to the frontend via the set callbacks.

### **`retro_serialize_size`**:
   * **Functionality**: Determines the size of the serialized state.
   * **Arguments**: None.
   * **Description**: The core communicates the size of the serialized state to the frontend.

### **`retro_serialize`**:
   * **Functionality**: Serializes the current state.
   * **Arguments**: A buffer and its size.
   * **Description**: The core fills the buffer with the serialized state, which can be saved by the frontend.

### **`retro_unserialize`**:
   * **Functionality**: Loads a serialized state.
   * **Arguments**: A buffer containing the serialized state and its size.
   * **Description**: The core loads the state from the buffer provided by the frontend.

### **`retro_cheat_reset`**:
   * **Functionality**: Resets cheats.
   * **Arguments**: None.
   * **Description**: Cheat configurations are reset internally within the core.

### **`retro_cheat_set`**:
   * **Functionality**: Sets a specific cheat.
   * **Arguments**: Index of the cheat, whether it's enabled, and the cheat code.
   * **Description**: The core sets the specified cheat based on the provided parameters.

### **`retro_load_game`**:
   * **Functionality**: Loads a game into the core.
   * **Arguments**: A pointer to a `retro_game_info` structure.
   * **Description**: The core loads the game based on the information provided in the `retro_game_info` structure.

### **`retro_load_game_special`**:
   * **Functionality**: Loads a special game type.
   * **Arguments**: Game type, a pointer to an array of `retro_game_info` structures, and the size of the array.
   * **Description**: The core loads the special game type based on the provided parameters.

### **`retro_unload_game`**:
   * **Functionality**: Unloads the game from the core.
   * **Arguments**: None.
   * **Description**: Unloading parameters and configurations are handled internally within the core.

### **`retro_get_region`**:
   * **Functionality**: Returns the region of the game.
   * **Arguments**: None.
   * **Description**: The core communicates the game's region to the frontend.

### **`retro_get_memory_data`**:
   * **Functionality**: Accesses specific memory data.
   * **Arguments**: Memory type ID.
   * **Description**: The core returns a pointer to the specified memory data.

### **`retro_get_memory_size`**:
   * **Functionality**: Gets the size of specific memory data.
   * **Arguments**: Memory type ID.
   * **Description**: The core communicates the size of the specified memory data to the frontend.

These functions form the bridge between the frontend and the core, allowing them to communicate and operate seamlessly. The data structures like `retro_system_info`, `retro_system_av_info`, and `retro_game_info` play a crucial role in this Description, providing structured data that both the frontend and core can understand and process.

## Data Structures

### **`retro_system_info`**

This structure provides general information about the core, such as its name, version, and supported file extensions.

- `library_name`: The name of the core/library.
- `library_version`: The version of the core.
- `valid_extensions`: A list of valid extensions for the core (e.g., "nes|fam").
- `need_fullpath`: Indicates if the core needs the full path to load content.
- `block_extract`: Indicates if the frontend should extract archives before loading.

### **`retro_system_av_info`**

   This structure provides audio and video specifications for the core, allowing the frontend to properly handle rendering and audio playback.
   - `timing`: Contains information about the core's FPS and audio sample rate.
   - `geometry`: Contains information about the core's base width, base height, max width, max height, and aspect ratio.

### **`retro_game_info`**
   
   This structure provides information about the game or content being loaded into the core. It can contain either a path to the content or the content data itself.

- `path`: The path to the game/content file.
- `data`: The actual game/content data.
- `size`: The size of the game/content data.
- `meta`: Metadata of the game/content.

### **`retro_input_descriptor`**
This structure provides a description of the input devices and their inputs, allowing the frontend to properly handle user input.
- `port`: The port on which the input device is connected.
- `device`: The type of device (e.g., joystick, keyboard).
- `index`: The index of the device (if there are multiple devices of the same type).
- `id`: The ID of the input (e.g., button A, button B).
- `description`: A description of the input.

### **`retro_variable`**
This structure is used for core options that can be set by the frontend or the user. It allows the core to communicate its settings and configurations.
- `key`: The key/name of the variable.
- `value`: The value of the variable.
