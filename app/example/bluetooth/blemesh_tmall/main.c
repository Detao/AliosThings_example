/* main.c - Application main entry point */

/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <aos/aos.h>
#include <aos/kernel.h>

#include <misc/printk.h>
#include "mesh_profile.h"

static void prov_complete(u16_t net_idx, u16_t addr)
{
    printk("Provisioning completed!\n");
    printk("Net ID: %u\n", net_idx);
    printk("Unicast addr: 0x%04x\n", addr);
}

static void prov_reset(void)
{
    printk("reset provisioning\n");
}

/* UUID for identifying the unprovisioned node */
/* sample value generated from IoT cloud for testing */
#if 0
static const uint8_t pid[4] = { 0x00, 0x02, 0xa4, 0xee }; // product id:173294
static const uint8_t mac_addr[6] = { 0xaa, 0xbb, 0xcc, 0x00, 0x00, 0x01 }; // device name
static const char secret[] = "zd6sCqghXY37ySE6gmryIVkzkqruV6Vz"; // device secret
#else
/* provided by tmall team for testing */
static const uint8_t pid[4] = { 0x00, 0x00, 0x00, 0x02 };
static const uint8_t mac_addr[6] = { 0x78, 0xda, 0x07, 0x11, 0x27, 0xff };
static const char secret[] = "9c98d67f6bd3be48d36bd8ed29a95bab";
#endif
static const char pkey[] = "b1HaDlKgwoV"; // product key

static uint8_t dev_uuid[16] = { 0x00 };
static uint8_t digest[32] = { 0x00 };

static void blemesh_tmall_profile(void)
{
    int ret;

    printk("Initializing tmall ble mesh profile...\n");

    // calculate digest of static oob info
    ble_mesh_calculate_digest(digest, pid, mac_addr, secret);
    printk("digest: %s\n", bt_hex(digest, 16));

    // construct uuid broadcasted in unprovisioned beacon
    ble_mesh_construct_uuid(dev_uuid, pid, mac_addr);
    printk("dev_uuid: %s\n", bt_hex(dev_uuid, 16));

    // configure mesh profile prov
    ret = ble_mesh_prov_init(dev_uuid, digest, 16,
                             prov_complete, prov_reset);
    if (ret != 0) {
        printk("fail to initialize mesh profile provisioning capability\n");
    }

    // configure mesh profile composition data page 0
    ret = ble_mesh_composition_data_init();
    if (ret != 0) {
        printk("fail to configure composistion data\n");
    }

    // configure mesh profile supported elements and models
    ret = ble_mesh_model_init();
    if (ret != 0) {
        printk("fail to initialize mesh profile supported elements and models\n");
    }

    // start mesh profile
    ret = ble_mesh_start();
    if (ret != 0) {
        printk("fail to initialize mesh profile\n");
    }
}

static void app_delayed_action(void *arg)
{
    blemesh_tmall_profile();
}

int application_start(int argc, char **argv)
{
    aos_post_delayed_action(1000, app_delayed_action, NULL);
    aos_loop_run();
    return 0;
}
