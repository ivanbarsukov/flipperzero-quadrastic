/*
 * Copyright 2025 Ivan Barsukov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../../engine/entity.h"

typedef void (*ContextMenuBackCallback)(void* context);
typedef void (*ContextMenuItemCallback)(void* context, uint32_t index);

void
context_menu_add_item(Entity* self,
                      const char* label,
                      uint32_t index,
                      ContextMenuItemCallback callback,
                      void* callback_context);

void
context_menu_back_callback_set(Entity* entity,
                               ContextMenuBackCallback back_callback,
                               void* callback_context);

void
context_menu_reset_state(Entity* entity);

extern const EntityDescription context_menu_description;
