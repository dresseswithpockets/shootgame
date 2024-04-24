#pragma once

struct VirtualAxisNodeKeys {
    KeyboardKey negative;
    KeyboardKey positive;
};

struct VirtualAxisNodeGamepadStickAxis {
    int gamepad_index;
    GamepadAxis axis;
    // TODO: deadzone? does raylib take care of that for us?
};

struct VirtualAxisNode {
    // 0 is nothing, 1 is keys, 2 is gamepad stick axis
    int node_kind;
    union {
        struct VirtualAxisNodeKeys keys;
        struct VirtualAxisNodeGamepadStickAxis gamepad_stick_axis;
        // TODO: gamepad left stick and right stick axis support
    };
};

typedef struct VirtualAxis {
    struct VirtualAxisNode nodes[8];

    // resulting axis value
    float value;
} VirtualAxis;

struct VirtualButtonNodeGampadButton {
    int gamepad_index;
    GamepadButton button;
};

struct VirtualButtonNode {
    // 0 is nothing, 1 is keyboard key, 2 is gamepad button
    // TODO: key-modified buttons?
    int node_kind;
    union {
        KeyboardKey key;
        struct VirtualButtonNodeGampadButton gamepad_button;
    };
};

typedef struct VirtualButton {
    struct VirtualButtonNode nodes[8];

    int counter;
    bool this_frame;
    bool is_down;
} VirtualButton;

int virtual_axis_add_keys(VirtualAxis* vaxis, KeyboardKey negative, KeyboardKey positive);
int virtual_axis_add_gamepad_stick(VirtualAxis* vaxis, int gamepad_index, GamepadAxis axis);
void virtual_axis_update(VirtualAxis* vaxis);

int virtual_button_add_key(VirtualButton* vbutton, KeyboardKey key);
int virtual_button_add_gamepad_button(VirtualButton* vbutton, int gamepad_index, GamepadButton button);
void virtual_button_update(VirtualButton* vbutton);
