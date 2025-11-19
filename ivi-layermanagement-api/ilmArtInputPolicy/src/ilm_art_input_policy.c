/**************************************************************************
 *
 * Copyright 2015 Codethink Ltd
 * Copyright (C) 2015 Advanced Driver Information Technology Joint Venture GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivi-art-input-policy-client-protocol.h"
#include "ilm_art_input_policy.h"
#include "ilm_control_platform.h"

extern struct ilm_control_context ilm_context;

static int verify_surface_id_exists(struct ilm_control_context *ctx,
                                     t_ilm_surface surfaceID)
{
    struct surface_context *surface_ctx = NULL;
    int surface_found = 0;

    /* Check if surface identifier is valid and bound to an existing surface */
    wl_list_for_each(surface_ctx, &ctx->wl.list_surface, link) {
        if (surface_ctx->id_surface == surfaceID) {
            surface_found = 1;
            break;
        }
    }

    return surface_found;
}

ILM_EXPORT ilmErrorTypes
ilm_art_addInputRectangle(t_ilm_surface surfaceID,
                         t_ilm_int x,
                         t_ilm_int y,
                         t_ilm_uint width,
                         t_ilm_uint height)
{
    struct ilm_control_context *ctx;
    ilmErrorTypes returnValue = ILM_FAILED;

    /* Acquire context */
    ctx = sync_and_acquire_instance();

    /* Check if surface identifier is valid and bound to an existing surface */
    if (verify_surface_id_exists(ctx, surfaceID) == 0) {
        fprintf(stderr, "surface ID %d not found\n", surfaceID);
    } else {

        /* Surface found. Lets forward the request to the compositor */
        ivi_art_input_policy_add_input_rectangle(
            ctx->wl.art_input_policy_controller,
            surfaceID,
            x,
            y,
            width,
            height
        );

        returnValue = ILM_SUCCESS;
    }

    /* Release context */
    release_instance();
    return returnValue;
}

ILM_EXPORT ilmErrorTypes
ilm_art_clearInputRegions(t_ilm_surface surfaceID) {
    struct ilm_control_context *ctx;
    ilmErrorTypes returnValue = ILM_FAILED;

    /* Acquire context */
    ctx = sync_and_acquire_instance();

    /* Check if surface identifier is valid and bound to an existing surface */
    if (verify_surface_id_exists(ctx, surfaceID) == 0) {
        fprintf(stderr, "surface ID %d not found\n", surfaceID);
    } else {

        /* Surface found. Lets forward the request to the compositor */
        ivi_art_input_policy_clear_input_regions(
            ctx->wl.art_input_policy_controller,
            surfaceID
        );

        returnValue = ILM_SUCCESS;
    }

    /* Release context */
    release_instance();
    return returnValue;
}

ILM_EXPORT ilmErrorTypes
ilm_art_resetInputRegions(t_ilm_surface surfaceID) {
    struct ilm_control_context *ctx;
    ilmErrorTypes returnValue = ILM_FAILED;

    /* Acquire context */
    ctx = sync_and_acquire_instance();

    /* Check if surface identifier is valid and bound to an existing surface */
    if (verify_surface_id_exists(ctx, surfaceID) == 0) {
        fprintf(stderr, "surface ID %d not found\n", surfaceID);
    } else {

        /* Surface found. Lets forward the request to the compositor */
        ivi_art_input_policy_reset_input_region(
            ctx->wl.art_input_policy_controller,
            surfaceID
        );

        returnValue = ILM_SUCCESS;
    }

    /* Release context */
    release_instance();
    return returnValue;
}

ILM_EXPORT ilmErrorTypes
ilm_art_commitInputRegion(t_ilm_surface surfaceID) {
    struct ilm_control_context *ctx;
    ilmErrorTypes returnValue = ILM_FAILED;

    /* Acquire context */
    ctx = sync_and_acquire_instance();

    /* Check if surface identifier is valid and bound to an existing surface */
    if (verify_surface_id_exists(ctx, surfaceID) == 0) {
        fprintf(stderr, "surface ID %d not found\n", surfaceID);
    } else {

        /* Surface found. Lets forward the request to the compositor */
        ivi_art_input_policy_commit_input_region(
            ctx->wl.art_input_policy_controller,
            surfaceID
        );

        returnValue = ILM_SUCCESS;
    }

    /* Release context */
    release_instance();
    return returnValue;
}