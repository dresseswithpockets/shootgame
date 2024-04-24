#include "math.h"
#include "input.h"

int virtual_axis_get_first_available_node(VirtualAxis* vaxis) {
    for (size_t i = 0; i < ARRAY_LEN(vaxis->nodes); i++) {
        if (vaxis->nodes[i].node_kind == 0) {
            return i;
        }
    }

    return -1;
}

int virtual_axis_add_keys(VirtualAxis* vaxis, KeyboardKey negative, KeyboardKey positive) {
    int node_idx = virtual_axis_get_first_available_node(vaxis);
    if (node_idx == -1)
        return -1;

    vaxis->nodes[node_idx] = (struct VirtualAxisNode) {
        .node_kind = 1,
        .keys = {
            .negative = negative,
            .positive = positive,
        }
    };
    return node_idx;
}

int virtual_axis_add_gamepad_stick(VirtualAxis* vaxis, int gamepad_index, GamepadAxis axis) {
    int node_idx = virtual_axis_get_first_available_node(vaxis);
    if (node_idx == -1)
        return -1;

    vaxis->nodes[node_idx] = (struct VirtualAxisNode) {
        .node_kind = 2,
        .gamepad_stick_axis = {
            .gamepad_index = gamepad_index,
            .axis = axis,
        }
    };
    return node_idx;
}

void virtual_axis_update(VirtualAxis* vaxis) {
    for (size_t i = 0; i < ARRAY_LEN(vaxis->nodes); i++) {
        struct VirtualAxisNode node = vaxis->nodes[i];

        float value = 0.0;
        switch (node.node_kind) {
        case 0: // none, skip
            continue;

        case 1: // keys
            if (IsKeyDown(node.keys.negative)) value -= 1.0;
            if (IsKeyDown(node.keys.positive)) value += 1.0;
            break;

        case 2: // gamepad stick axis
            // TODO: deadzone?
            value = GetGamepadAxisMovement(node.gamepad_stick_axis.gamepad_index, node.gamepad_stick_axis.axis);
            break;
        }

        // just take the first non-zero value
        if (value != 0.0) {
            vaxis->value = value;
            return;
        }
    }

    vaxis->value = 0.0;
}

int virtual_button_get_first_available_node(VirtualButton* vbutton) {
    for (size_t i = 0; i < ARRAY_LEN(vbutton->nodes); i++) {
        if (vbutton->nodes[i].node_kind == 0) {
            return i;
        }
    }

    return -1;
}

int virtual_button_add_key(VirtualButton* vbutton, KeyboardKey key) {
    int node_idx = virtual_button_get_first_available_node(vbutton);
    if (node_idx == -1)
        return -1;

    vbutton->nodes[node_idx] = (struct VirtualButtonNode) {
        .node_kind = 1,
        .key = key,
    };
    return node_idx;
}

int virtual_button_add_gamepad_button(VirtualButton* vbutton, int gamepad_index, GamepadButton button) {
    int node_idx = virtual_button_get_first_available_node(vbutton);
    if (node_idx == -1)
        return -1;

    vbutton->nodes[node_idx] = (struct VirtualButtonNode) {
        .node_kind = 1,
        .gamepad_button = {
            .gamepad_index = gamepad_index,
            .button = button,
        },
    };
    return node_idx;
}

void virtual_button_update(VirtualButton* vbutton) {
    for (size_t i = 0; i < ARRAY_LEN(vbutton->nodes); i++) {
        struct VirtualButtonNode node = vbutton->nodes[i];

        bool new_button_down = vbutton->is_down;
        switch (node.node_kind) {
        case 0: // none, skip
            continue;

        case 1: // key
            new_button_down = IsKeyDown(node.key);
            break;

        case 2: // gamepad button
            new_button_down = IsGamepadButtonDown(node.gamepad_button.gamepad_index, node.gamepad_button.button);
            break;
        }

        if (new_button_down != vbutton->is_down) {
            vbutton->counter = 0;
            vbutton->this_frame = true;
        } else {
            vbutton->counter += 1;
            vbutton->this_frame = false;
        }

        vbutton->is_down = new_button_down;
    }
}
