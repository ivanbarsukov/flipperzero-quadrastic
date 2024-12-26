#include "input_converter.h"

#include <furi/core/message_queue.h>

#include "../game.h"

// #define FRAME_IN_MS (1000 / 30)
// #define CHECK_PERIOD_IN_MS 150

#define CHECK_PERIOD_IN_FRAMES 5 // (int)(CHECK_PERIOD_IN_MS / FRAME_IN_MS)
#define LONG_PRESS_IN_FRAMES 10  // (int)(CHECK_PERIOD_IN_MS * 2 / FRAME_IN_MS)

const InputKey input_keys[InputKeyMAX] = { InputKeyUp,    InputKeyDown,
                                           InputKeyRight, InputKeyLeft,
                                           InputKeyOk,    InputKeyBack };

const GameKey game_keys[InputKeyMAX] = {
    GameKeyUp, GameKeyDown, GameKeyRight, GameKeyLeft, GameKeyOk, GameKeyBack
};

struct InputConverter
{
    size_t holded_frames_for_key[InputKeyMAX];
    FuriMessageQueue* queue;
};

static void
put_input_event(InputConverter* input_converter, InputEvent* event)
{
    FURI_LOG_D(GAME_NAME,
               "Input event: %s %s",
               input_get_key_name(event->key),
               input_get_type_name(event->type));

    FuriStatus status =
      furi_message_queue_put(input_converter->queue, event, 0);
    furi_check(status == FuriStatusOk);
}

static void
process_pressed(InputConverter* input_converter,
                InputState* input_state,
                GameKey game_key,
                InputKey input_key)
{
    if (!(input_state->pressed & game_key)) {
        return;
    }

    ++(input_converter->holded_frames_for_key[input_key]);

    InputEvent event = { .key = input_key, .type = InputTypePress };
    put_input_event(input_converter, &event);
}

static void
process_holded(InputConverter* input_converter,
               InputState* input_state,
               GameKey game_key,
               InputKey input_key)
{
    size_t* holded_frames_for_key =
      &input_converter->holded_frames_for_key[input_key];

    if (!(input_state->held & game_key) || *holded_frames_for_key == 0) {
        return;
    }

    // Add current frame
    ++(*holded_frames_for_key);

    InputEvent event = { .key = input_key };
    if (*holded_frames_for_key == LONG_PRESS_IN_FRAMES) {
        // Long press
        event.type = InputTypeLong;
        put_input_event(input_converter, &event);
    } else if (*holded_frames_for_key ==
               LONG_PRESS_IN_FRAMES + CHECK_PERIOD_IN_FRAMES) {
        // Pause between events
        input_converter->holded_frames_for_key[input_key] =
          LONG_PRESS_IN_FRAMES;

        // Repeat
        event.type = InputTypeRepeat;
        put_input_event(input_converter, &event);
    }
}

static void
process_released(InputConverter* input_converter,
                 InputState* input_state,
                 GameKey game_key,
                 InputKey input_key)
{
    size_t* holded_frames_for_key =
      &input_converter->holded_frames_for_key[input_key];

    if (!(input_state->released & game_key) || 0 == *holded_frames_for_key) {
        return;
    }

    InputEvent event = { .key = input_key };

    // Process short press
    if (*holded_frames_for_key < LONG_PRESS_IN_FRAMES) {
        event.type = InputTypeShort;
        put_input_event(input_converter, &event);
    }

    // Reset state
    input_converter->holded_frames_for_key[input_key] = 0;

    // Put release event
    event.type = InputTypeRelease;
    put_input_event(input_converter, &event);
}

InputConverter*
input_converter_alloc(void)
{
    InputConverter* input_converter = malloc(sizeof(InputConverter));

    // Init queue
    input_converter->queue =
      furi_message_queue_alloc(2 * InputKeyMAX, sizeof(InputEvent));

    // Init counter
    for (size_t i = 0; i < InputKeyMAX; ++i) {
        input_converter->holded_frames_for_key[i] = 0;
    }

    return input_converter;
}

void
input_converter_free(InputConverter* input_converter)
{
    furi_check(input_converter);
    furi_message_queue_free(input_converter->queue);
    free(input_converter);
}

void
input_converter_process_state(InputConverter* input_converter,
                              InputState* input_state)
{
    furi_check(input_converter);
    furi_check(input_state);

    // Process new state
    for (size_t key_index = 0; key_index < sizeof(game_keys); ++key_index) {
        GameKey game_key = game_keys[key_index];
        InputKey input_key = input_keys[key_index];
        process_pressed(input_converter, input_state, game_key, input_key);
        process_holded(input_converter, input_state, game_key, input_key);
        process_released(input_converter, input_state, game_key, input_key);
    }
}

FuriStatus
input_converter_get_event(InputConverter* input_converter, InputEvent* event)
{
    furi_check(input_converter);
    return furi_message_queue_get(input_converter->queue, event, 0);
}
