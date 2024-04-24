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
    // 0 is keys, 1 is gamepad stick axis
    int node_kind;
    union {
        struct VirtualAxisNodeKeys keys;
        struct VirtualAxisNodeGamepadStickAxis gamepad_stick_axis;
        // TODO: gamepad left stick and right stick axis support
    };
};

typedef struct VirtualAxis {
    struct VirtualAxisNode *nodes;
    int node_count;

    // resulting axis value
    float value;
} VirtualAxis;

struct VirtualButtonNodeGampadButton {
    int gamepad_index;
    GamepadButton button;
};

struct VirtualButtonNode {
    // 0 is keyboard key, 1 is gamepad button
    // TODO: key-modified buttons?
    int node_kind;
    union {
        KeyboardKey key;
        struct VirtualButtonNodeGampadButton gamepad_button;
    };
};

typedef struct VirtualButton {
    struct VirtualButtonNode *nodes;
    int node_count;

    int counter;
    bool this_frame;
    bool is_down;
} VirtualButton;

void virtual_axis_update(VirtualAxis* vaxis);
void virtual_button_update(VirtualButton* vbutton);
