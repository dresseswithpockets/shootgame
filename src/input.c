#include "input.h"

void virtual_axis_update(VirtualAxis* vaxis) {
    for (int i = 0; i < vaxis->node_count; i++) {
        struct VirtualAxisNode node = vaxis->nodes[i];

        float value = 0.0;
        switch (node.node_kind) {
        case 0: // keys
            if (IsKeyDown(node.keys.negative)) value -= 1.0;
            if (IsKeyDown(node.keys.positive)) value += 1.0;
            break;
        case 1: // gamepad stick axis
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

void virtual_button_update(VirtualButton* vbutton) {
    for (int i = 0; i < vbutton->node_count; i++) {
        struct VirtualButtonNode node = vbutton->nodes[i];

        bool new_button_down = vbutton->is_down;
        switch (node.node_kind) {
        case 0: // key
            new_button_down = IsKeyDown(node.key);
            break;

        case 1: // gamepad button
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
