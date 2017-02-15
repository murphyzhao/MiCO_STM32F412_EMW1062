/**
 ******************************************************************************
 * @file    mico_system_init.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-May-2014
 * @brief   This file provide the mico system initialize function.
 ******************************************************************************
 *
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

#include <time.h>

#include "mico.h"

#include "system_internal.h"

#ifdef AIRKISS_DISCOVERY_ENABLE
#include "easylink/airkiss_discovery.h"
#endif

extern system_context_t* sys_context;

/******************************************************
 *               Variables Definitions
 ******************************************************/
#ifdef MICO_WLAN_CONNECTION_ENABLE
static OSStatus system_config_mode_start( system_context_t* in_context )
{
    OSStatus err = kNoErr;
#if (MICO_WLAN_CONFIG_MODE == CONFIG_MODE_EASYLINK) || \
    (MICO_WLAN_CONFIG_MODE == CONFIG_MODE_SOFT_AP) ||  \
    (MICO_WLAN_CONFIG_MODE == CONFIG_MODE_USER) ||  \
    (MICO_WLAN_CONFIG_MODE == CONFIG_MODE_EASYLINK_WITH_SOFTAP)
    err = system_easylink_start( in_context );
    require_noerr( err, exit );
#elif ( MICO_WLAN_CONFIG_MODE == CONFIG_MODE_WAC)
    err = system_easylink_wac_start( in_context );
    require_noerr( err, exit );
#else
    #error "Wi-Fi configuration mode is not defined"
#endif
exit:
    return err;
}
#endif

OSStatus mico_system_init( mico_Context_t* in_context )
{
  OSStatus err = kNoErr;

  require_action( in_context, exit, err = kNotPreparedErr );

  /* Initialize mico notify system */
  err = system_notification_init( sys_context );
  require_noerr( err, exit ); 

#ifdef MICO_SYSTEM_MONITOR_ENABLE
  /* MiCO system monitor */
  err = mico_system_monitor_daemen_start( );
  require_noerr( err, exit ); 
#endif

#ifdef MICO_CLI_ENABLE
  /* MiCO command line interface */
  cli_init();
#endif

  /* Network PHY driver and tcp/ip static init */
  err = system_network_daemen_start( sys_context );
  require_noerr( err, exit ); 

#ifdef MICO_WLAN_CONNECTION_ENABLE
  if( sys_context->flashContentInRam.micoSystemConfig.configured == unConfigured){
#if (MICO_WLAN_CONFIG_MODE_TRIGGER) &&  (MICO_WLAN_CONFIG_MODE_TRIGGER != CONFIG_MODE_TRIGGER_AUTO )
    system_log("Empty configuration. Start configuration mode by external trigger");
    micoWlanPowerOff();
#else
    system_log("Empty configuration. Starting configuration mode...");
    err = system_config_mode_start( sys_context );
    require_noerr( err, exit );
#endif
  }
  else if( sys_context->flashContentInRam.micoSystemConfig.configured == unConfigured2 ){
      system_log("Empty configuration. Starting configuration mode by external trigger");
      err = system_config_mode_start( sys_context );
      require_noerr( err, exit );
  }
  else if( sys_context->flashContentInRam.micoSystemConfig.configured == wLanUnConfigured ){
      system_log("Re-config wlan configuration. Starting configuration mode...");
      err = system_config_mode_start( sys_context );
      require_noerr( err, exit );
  }
#ifdef MFG_MODE_AUTO
  else if( sys_context->flashContentInRam.micoSystemConfig.configured == mfgConfigured ){
    system_log( "Enter MFG mode automatically" );
    mico_mfg_test( in_context );
    mico_thread_sleep( MICO_NEVER_TIMEOUT );
  }
#endif
  else{
    system_log("Available configuration. Starting Wi-Fi connection...");
    system_connect_wifi_fast( sys_context );
  }
#endif
  
  /* System discovery */
#ifdef MICO_SYSTEM_DISCOVERY_ENABLE
  system_discovery_init( sys_context );
#endif
  
  /*Local configuration server*/
#ifdef MICO_CONFIG_SERVER_ENABLE
  config_server_start( );
#endif
  
#ifdef AIRKISS_DISCOVERY_ENABLE
  err = airkiss_discovery_start( AIRKISS_APP_ID, AIRKISS_DEVICE_ID );
  require_noerr( err, exit );
#endif

exit:
  return err;
}


