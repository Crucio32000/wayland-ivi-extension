/** @file       ivi-input-policy-controller.h
*   @brief      Module ivi-input-policy-controller - Implementation
*     
*   @author     Domenico Nicita
*     
*   @copyright
*               Copyright 2025 - ART spa.
*               All rights reserved.
*               This file is copyrighted and the property of ART spa.
*               It contains confidential and proprietary information. Any copies of
*               this file (in whole or in part) may only be used subject to prior
*               written permission from ART spa.
*
*   @note       Module based on the Template "SWC_Template_C_Language" version "1.1.0"
***********************************************************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <weston.h>
#include <ivi-layout-export.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libweston/plugin-registry.h>
#include "ilm_types.h"

#include "ivi-input-policy-server-protocol.h"
#include "ivi-controller.h"

#define PRIV_WLOG(fmt, ...) \
    weston_log("ivi-input-policy-controller: " fmt, ##__VA_ARGS__)

struct art_input_policy_context {
    struct ivishell* ivishell;

    struct wl_listener surface_created;
    struct wl_listener surface_destroyed;
    struct wl_listener shell_destroy_listener;

    struct wl_list resource_list;   /* Resource is a client that bound to our controller */

    struct wl_event_source *debug_timer;
    uint8_t debug_timer_enabled;
    uint32_t debug_timer_interval_ms; /* in milliseconds */

    uint8_t request_log_enabled;
};

static void input_policy_controller_deinit(struct art_input_policy_context *ctx) {
    if (NULL != ctx) {
        struct wl_resource *resource, *tmp_resource;
        wl_list_remove(&ctx->surface_created.link);
        wl_list_remove(&ctx->surface_destroyed.link);
        wl_list_remove(&ctx->shell_destroy_listener.link);

        if (ctx->debug_timer != NULL) {
            wl_event_source_remove(ctx->debug_timer);
        }

        wl_resource_for_each_safe(resource, tmp_resource, &ctx->resource_list) {
            /*We have set destroy function for this resource.
            * The below api will call unbind_resource_controller and
            * free up the controller structure*/
            wl_resource_destroy(resource);
        }

        free(ctx);
    }
}

static void input_policy_controller_destroy(struct wl_listener *listener, void *data)
{
    if (NULL != listener) {
        struct art_input_policy_context *ctx =
        wl_container_of(listener, ctx, shell_destroy_listener);

        input_policy_controller_deinit(ctx);
    }
}

static void
handle_surface_create(struct wl_listener *listener, void *data)
{
    struct art_input_policy_context *ctx =
        wl_container_of(listener, ctx, surface_created);

    /* Pull some data from the created surface */
    struct ivisurface *surf = (struct ivisurface *) data;
    struct ivi_layout_surface *layout_surface = surf->layout_surface;
    const struct ivi_layout_interface *interface =
        ctx->ivishell->interface;
    struct weston_surface *w_surf = interface->surface_get_weston_surface(layout_surface);

    PRIV_WLOG("surface %p (weston surface %p) created\n", surf, w_surf);

}

static void
handle_surface_destroy(struct wl_listener *listener, void *data)
{
    struct art_input_policy_context *ctx =
            wl_container_of(listener, ctx, surface_destroyed);
    struct ivisurface *surf = (struct ivisurface *) data;

    PRIV_WLOG("surface %p destroyed\n", surf);
}


static struct ivisurface *
get_ivi_surf_from_ivi_layout_ctx(struct art_input_policy_context *ctx,
        struct ivi_layout_surface *lyt_surf)
{
    struct ivisurface *surf_ctx = NULL;
    struct ivisurface *ret_ctx = NULL;
    wl_list_for_each(surf_ctx, &ctx->ivishell->list_surface, link) {
        if (lyt_surf == surf_ctx->layout_surface) {
            ret_ctx = surf_ctx;
            break;
        }
    }
    return ret_ctx;
}

static struct ivisurface *
get_ivi_surf_from_id(struct art_input_policy_context *ctx,
        uint32_t ivi_surf_id)
{
    const struct ivi_layout_interface *interface = ctx->ivishell->interface;
    struct ivi_layout_surface *lyt_surf;
    lyt_surf = interface->get_surface_from_id(ivi_surf_id);

    return get_ivi_surf_from_ivi_layout_ctx(ctx, lyt_surf);
}

static struct weston_surface* 
get_west_surface_from_id(struct art_input_policy_context *ctx,
        uint32_t ivi_surf_id)
{
    struct ivisurface* ivi_surf = NULL;
    struct ivi_layout_surface *layout_surface = NULL;
    struct weston_surface *west_surf = NULL;
    const struct ivi_layout_interface *interface =
        ctx->ivishell->interface;

    /* Retrieve IVI Surface from Identifier */
    ivi_surf = get_ivi_surf_from_id(ctx, ivi_surf_id);

    if (NULL == ivi_surf)
        return NULL;

    layout_surface = ivi_surf->layout_surface;

    west_surf = interface->surface_get_weston_surface(layout_surface);

    return west_surf;
}

static void dump_input_region_from_surface(struct weston_surface *w_surf, uint8_t dump_current_region)
{
    pixman_region32_t *input_region;
    pixman_box32_t *rects;
    int n_rects, i;

    if (dump_current_region) {
        input_region = &w_surf->input;
    } else {
        input_region = &w_surf->pending.input;
    }

    rects = pixman_region32_rectangles(input_region, &n_rects);
    
    PRIV_WLOG("   %s Input region: %d rectangle(s)\n", dump_current_region ? "Current" : "Pending", n_rects);
    for (i = 0; i < n_rects; i++) {
        PRIV_WLOG("     Rect %d: x1=%d, y1=%d, x2=%d, y2=%d (width=%d, height=%d)\n",
                  i, rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2,
                  rects[i].x2 - rects[i].x1, rects[i].y2 - rects[i].y1);
    }
}

static int debug_timer_callback(void *data)
{
    struct art_input_policy_context *ctx = (struct art_input_policy_context *)data;
    struct ivisurface *surf;

    PRIV_WLOG("Debug Timer Callback: Iterating over surfaces\n");

    wl_list_for_each(surf, &ctx->ivishell->list_surface, link) {
        struct ivi_layout_surface *layout_surface = surf->layout_surface;
        const struct ivi_layout_interface *interface =
            ctx->ivishell->interface;
        struct weston_surface *w_surf = interface->surface_get_weston_surface(layout_surface);
        uint32_t surf_id = interface->get_id_of_surface(layout_surface);

        PRIV_WLOG(" Surface %p (weston surface %p) (SID: %u) Type %d\n",
                  surf, w_surf, surf_id, surf->type);

        if (w_surf != NULL) {            
            dump_input_region_from_surface(w_surf, 1); /* current region */
        }
    }
    /* Refresh timer */
    wl_event_source_timer_update(ctx->debug_timer, ctx->debug_timer_interval_ms);
    return 1;
}

static void read_config_from_weston(struct art_input_policy_context *ctx)
{
    /* Check weston config */
    struct weston_config *config = wet_get_config(ctx->ivishell->compositor);
    struct weston_config_section *section;

    section = weston_config_get_section(config, "ivi-input-policy-controller", NULL, NULL);
    if (section != NULL) {
        int enabled = 0;
        int interval_ms = 5000; /* default 5 seconds */

        if (weston_config_section_get_int(section,
                    "debug_timer_enabled", &enabled, 0) == 0) {
            ctx->debug_timer_enabled = (enabled != 0) ? 1 : 0;
        }

        if (weston_config_section_get_int(section,
                    "debug_timer_interval_ms", &interval_ms, 5000) == 0) {
            ctx->debug_timer_interval_ms = (uint32_t)interval_ms;
        }

        if (weston_config_section_get_int(section,
                    "request_log_enabled", &enabled, 0) == 0) {
            ctx->request_log_enabled = (enabled != 0) ? 1 : 0;
        }
    } else {
        PRIV_WLOG("No ivi-input-policy-controller section in weston config. Using default values.\n");
    }

}

static struct art_input_policy_context * create_context(struct ivishell *shell)
{
    struct art_input_policy_context *ctx = NULL;
    struct wl_event_loop* loop = NULL;

    ctx = calloc(1, sizeof *ctx);
    if (ctx == NULL) {
        PRIV_WLOG("%s: Failed to allocate memory for input context\n",
                   __FUNCTION__);
        return NULL;
    }

    ctx->ivishell = shell;

    /* Add signal handlers for ivi surfaces. */
    ctx->surface_created.notify = handle_surface_create;
    ctx->surface_destroyed.notify = handle_surface_destroy;

    wl_signal_add(&ctx->ivishell->ivisurface_created_signal, &ctx->surface_created);
    wl_signal_add(&ctx->ivishell->ivisurface_removed_signal, &ctx->surface_destroyed);
    ctx->ivishell->interface->shell_add_destroy_listener_once(
            &ctx->shell_destroy_listener, input_policy_controller_destroy);

    /* Check weston config */
    read_config_from_weston(ctx);

    /* Check if we need to enable debug timer */
    if (ctx->debug_timer_enabled) {
        PRIV_WLOG("Debug timer enabled, interval %u ms\n",
                  ctx->debug_timer_interval_ms);

        loop = wl_display_get_event_loop(shell->compositor->wl_display);
        ctx->debug_timer = wl_event_loop_add_timer(loop, debug_timer_callback, ctx);
        if (ctx->debug_timer != NULL) {
            wl_event_source_timer_update(ctx->debug_timer, ctx->debug_timer_interval_ms);
        } else {
            PRIV_WLOG("Failed to create debug timer\n");
        }
    } else {
        PRIV_WLOG("Debug timer disabled\n");
    }

    return ctx;
}

/*
 * Necessary functions / structures for registering a Wayland Global
 */

void impl_add_input_rectangle(struct wl_client *client,
				    struct wl_resource *resource,
				    uint32_t surface_id,
				    int32_t x,
				    int32_t y,
				    int32_t width,
				    int32_t height) 
{
    struct art_input_policy_context *ctx = wl_resource_get_user_data(resource);
    struct weston_surface *west_surf = get_west_surface_from_id(ctx, surface_id);

    if (ctx->request_log_enabled) {
        PRIV_WLOG("Client requested to add input rectangle to surface ID %u: x=%d, y=%d, width=%d, height=%d\n",
                  surface_id, x, y, width, height);
    }

    /* Add input rectangle*/
    /* Verify region is already initialized, otherwise do it now */
    if (pixman_region32_not_empty(&west_surf->pending.input) == 0) {
        pixman_region32_init(&west_surf->pending.input);
    }
    pixman_region32_union_rect(&west_surf->pending.input,
                               &west_surf->pending.input,
                               x, y, width, height);
}

void impl_clear_input_regions(struct wl_client *client,
                struct wl_resource *resource,
                uint32_t surface_id) 
{
    struct art_input_policy_context *ctx = wl_resource_get_user_data(resource);
    struct weston_surface *west_surf = get_west_surface_from_id(ctx, surface_id);

    if (ctx->request_log_enabled) {
        PRIV_WLOG("Client requested to clear input regions for surface ID %u\n", surface_id);
    }

    /* Clear input region */
    pixman_region32_fini(&west_surf->pending.input);
    pixman_region32_init(&west_surf->pending.input);
}

void impl_reset_input_region(struct wl_client *client,
                struct wl_resource *resource,
                uint32_t surface_id)
{
    struct art_input_policy_context *ctx = wl_resource_get_user_data(resource);
    struct weston_surface *west_surf = get_west_surface_from_id(ctx, surface_id);

    if (ctx->request_log_enabled) {
        PRIV_WLOG("Client requested to reset input region for surface ID %u\n", surface_id);
    }

    /* Reset input region to full surface */
    /* Snippet taken from libweston/compositor.c */
    pixman_region32_fini(&west_surf->pending.input);
    pixman_region32_init_rect(&west_surf->pending.input, INT32_MIN, INT32_MIN,
				  UINT32_MAX, UINT32_MAX);
}

void impl_commit_input_region(struct wl_client *client,
                struct wl_resource *resource,
                uint32_t surface_id)
{
    struct art_input_policy_context *ctx = wl_resource_get_user_data(resource);
    struct weston_surface *west_surf = get_west_surface_from_id(ctx, surface_id);

    if (ctx->request_log_enabled) {
        PRIV_WLOG("Client requested to commit input region changes for surface ID %u\n", surface_id);
        dump_input_region_from_surface(west_surf, 0); /* pending region */
    }
    
    /* Commit the changes by indicating that pending region is DIRTY */
    west_surf->pending.status |= WESTON_SURFACE_DIRTY_INPUT;
    ctx->ivishell->interface->commit_changes();
}

static const struct ivi_input_policy_interface art_input_implementation = {
    .add_input_rectangle = impl_add_input_rectangle,
    .clear_input_regions = impl_clear_input_regions,
    .reset_input_region = impl_reset_input_region,
    .commit_input_region = impl_commit_input_region,
};

static void
unbind_resource_controller(struct wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

static void
bind_ivi_input_policy(struct wl_client *client, void *data,
               uint32_t version, uint32_t id)
{
    struct art_input_policy_context *ctx = (struct art_input_policy_context *)data;
    struct wl_resource *resource;

    /* We should check version here. Client may bind to a wrong version */

    /* Implementation based on https://wayland-book.com/registry/server-side.html */
    resource = wl_resource_create(client,
                                  &ivi_input_policy_interface,
                                  1, id);
    
    /* Set Implementation */
    wl_resource_set_implementation(resource, &art_input_implementation,
        ctx, unbind_resource_controller);

    wl_list_insert(&ctx->resource_list, wl_resource_get_link(resource));

    /* Here you can put some communication to clients */
}

WL_EXPORT int
art_input_policy_module_init(struct ivishell *shell)
{
    int ret = -1;
    struct art_input_policy_context *ctx = NULL;

    ctx = create_context(shell);

    if (ctx != NULL) {
        /* Initialize Resource List (Remote Clients) */
        wl_list_init(&ctx->resource_list);

        /* Create Wayland Global for ivi_input_policy */
        if (wl_global_create(shell->compositor->wl_display,
                              &ivi_input_policy_interface, 1,
                              ctx, bind_ivi_input_policy) != NULL) {
            ret = 0;
        } else {
            PRIV_WLOG("Failed to create ivi_input_policy global\n");
        }
    }

    /* Handle errors */
    if (ret != 0) {
        PRIV_WLOG("Failed to initialize art input policy controller module\n");
        input_policy_controller_deinit(ctx);
        ctx = NULL;
    }

    return ret;
}

