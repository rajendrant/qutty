#ifndef QTCONFIGTAG_H
#define QTCONFIGTAG_H

#ifdef __cplusplus
extern "C" {
#endif

struct config_tag {
#define QUTTY_SERIALIZE_STRUCT_CONFIG_ELEMENT_LIST                                                                                                                                                 \
  QUTTY_SERIALIZE_ELEMENT_ARRAY(char, config_name, 100)                                                                                                                                            \
  /* Basic options */                                                                                                                                                                              \
  QUTTY_SERIALIZE_ELEMENT_ARRAY(char, host, 512)                                                                                                                                                   \
  int(port) int(protocol) int(addressfamily) int(close_on_exit) int(warn_on_close) int(                                                                                                            \
      ping_interval) /* in seconds */                                                                                                                                                              \
      int(tcp_nodelay) int(tcp_keepalives) QUTTY_SERIALIZE_ELEMENT_ARRAY(                                                                                                                          \
          char, loghost,                                                                                                                                                                           \
          512) /* logical host being contacted, for host key check */ /* Proxy options */                                                                                                          \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, proxy_exclude_list, 512) int(proxy_dns) int(                                                                                                             \
          even_proxy_localhost) int(proxy_type) QUTTY_SERIALIZE_ELEMENT_ARRAY(char, proxy_host,                                                                                                    \
                                                                              512) int(proxy_port)                                                                                                 \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(char, proxy_username, 128)                                                                                                                                 \
              QUTTY_SERIALIZE_ELEMENT_ARRAY(char, proxy_password, 128)                                                                                                                             \
                  QUTTY_SERIALIZE_ELEMENT_ARRAY(char, proxy_telnet_command, 512) /* SSH options */                                                                                                 \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, remote_cmd, 512) int(nopty) int(compression)                                                                                                             \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(int, ssh_kexlist,                                                                                                                                          \
                                        KEX_MAX) int(ssh_rekey_time) /* in minutes */                                                                                                              \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, ssh_rekey_data, 16) int(tryagent) int(agentfwd) int(                                                                                                     \
          change_username) /* allow username switching in SSH-2 */                                                                                                                                 \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(int, ssh_cipherlist, CIPHER_MAX)                                                                                                                               \
          Filename(keyfile) int(sshprot) /* use v1 or v2 when both available */                                                                                                                    \
      int(ssh2_des_cbc)                  /* "des-cbc" unrecommended SSH-2 cipher */                                                                                                                \
      int(ssh_no_userauth)               /* bypass "ssh-userauth" (SSH-2 only) */                                                                                                                  \
      int(ssh_show_banner)               /* show USERAUTH_BANNERs (SSH-2 only) */                                                                                                                  \
      int(try_tis_auth) int(try_ki_auth) int(try_gssapi_auth) /* attempt gssapi auth */                                                                                                            \
      int(gssapifwd)                                          /* forward tgt via gss */                                                                                                            \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(int, ssh_gsslist, 4) /* preference order for local GSS libs */                                                                                                 \
      Filename(ssh_gss_custom) int(ssh_subsys) /* run a subsystem rather than a command */                                                                                                         \
      int(ssh_subsys2)                         /* fallback to go with remote_cmd_ptr2 */                                                                                                           \
      int(ssh_no_shell)                        /* avoid running a shell */                                                                                                                         \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, ssh_nc_host, 512)  /* host to connect to in `nc' mode */                                                                                                 \
      int(ssh_nc_port) /* port to connect to in `nc' mode */ /* Telnet options */                                                                                                                  \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, termtype, 32)                                                                                                                                            \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(char, termspeed, 32)                                                                                                                                       \
              QUTTY_SERIALIZE_ELEMENT_ARRAY(char, ttymodes, 768) /* MODE\tVvalue\0MODE\tA\0\0 */                                                                                                   \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, environmt, 1024)       /* VAR\tvalue\0VAR\tvalue\0\0 */                                                                                                  \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, username, 100) int(username_from_env)                                                                                                                    \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(char, localusername, 100) int(rfc_environ) int(                                                                                                            \
              passive_telnet) /* Serial port options */                                                                                                                                            \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, serline, 256) int(serspeed) int(serdatabits) int(                                                                                                        \
          serstopbits) int(serparity) int(serflow) /* Keyboard options */                                                                                                                          \
      int(bksp_is_delete) int(rxvt_homeend) int(funky_type) int(                                                                                                                                   \
          no_applic_c)          /* totally disable app cursor keys */                                                                                                                              \
      int(no_applic_k)          /* totally disable app keypad */                                                                                                                                   \
      int(no_mouse_rep)         /* totally disable mouse reporting */                                                                                                                              \
      int(no_remote_resize)     /* disable remote resizing */                                                                                                                                      \
      int(no_alt_screen)        /* disable alternate screen */                                                                                                                                     \
      int(no_remote_wintitle)   /* disable remote retitling */                                                                                                                                     \
      int(no_dbackspace)        /* disable destructive backspace */                                                                                                                                \
      int(no_remote_charset)    /* disable remote charset config */                                                                                                                                \
      int(remote_qtitle_action) /* remote win title query action */                                                                                                                                \
      int(app_cursor) int(app_keypad) int(nethack_keypad) int(telnet_keyboard) int(                                                                                                                \
          telnet_newline) int(alt_f4) /* is it special? */                                                                                                                                         \
      int(alt_space)                  /* is it special? */                                                                                                                                         \
      int(alt_only)                   /* is it special? */                                                                                                                                         \
      int(localecho) int(localedit) int(alwaysontop) int(fullscreenonaltenter) int(scroll_on_key) int(                                                                                             \
          scroll_on_disp) int(erase_to_scrollback) int(compose_key) int(ctrlaltkeys)                                                                                                               \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(char, wintitle,                                                                                                                                            \
                                        256) /* initial window title */ /* Terminal options */                                                                                                     \
      int(savelines) int(dec_om) int(wrap_mode) int(lfhascr) int(                                                                                                                                  \
          cursor_type)                                    /* 0=block 1=underline 2=vertical */                                                                                                     \
      int(blink_cur) int(beep) int(beep_ind) int(bellovl) /* bell overload protection active? */                                                                                                   \
      int(bellovl_n)                                      /* number of bells to cause overload */                                                                                                  \
      int(bellovl_t) /* time interval for overload (seconds) */                                                                                                                                    \
      int(bellovl_s) /* period of silence to re-enable bell (s) */                                                                                                                                 \
      Filename(bell_wavefile) int(scrollbar) int(scrollbar_in_fullscreen) int(resize_action) int(                                                                                                  \
          bce) int(blinktext) int(win_name_always) int(width) int(height)                                                                                                                          \
          FontSpec(font) int(font_quality)                                                                                                                                                         \
              Filename(logfilename) int(logtype) int(logxfovr) int(logflush) int(logomitpass) int(                                                                                                 \
                  logomitdata) int(hide_mouseptr) int(sunken_edge) int(window_border)                                                                                                              \
                  QUTTY_SERIALIZE_ELEMENT_ARRAY(char, answerback, 256)                                                                                                                             \
                      QUTTY_SERIALIZE_ELEMENT_ARRAY(                                                                                                                                               \
                          char, printer, 128) int(arabicshaping) int(bidi) /* Colour options */                                                                                                    \
      int(ansi_colour) int(xterm_256_colour) int(system_colour) int(try_palette) int(                                                                                                              \
          bold_colour) /* Selection options */                                                                                                                                                     \
      int(mouse_is_xterm) int(rect_select) int(rawcnp) int(rtf_paste) int(mouse_override)                                                                                                          \
          QUTTY_SERIALIZE_ELEMENT_ARRAY(short, wordness, 256) /* translations */                                                                                                                   \
      int(vtmode) QUTTY_SERIALIZE_ELEMENT_ARRAY(char, line_codepage, 128) int(cjk_ambig_wide) int(                                                                                                 \
          utf8_override) int(xlat_capslockcyr) /* X11 forwarding */                                                                                                                                \
      int(x11_forward) QUTTY_SERIALIZE_ELEMENT_ARRAY(char, x11_display, 128) int(x11_auth)                                                                                                         \
          Filename(xauthfile) /* port forwarding */                                                                                                                                                \
      int(lport_acceptall)    /* accept conns from hosts other than localhost */                                                                                                                   \
      int(rport_acceptall)    /* same for remote forwarded ports (SSH-2 only) */                                                                                                                   \
      QUTTY_SERIALIZE_ELEMENT_ARRAY(char, portfwd, 1024) /* SSH bug compatibility modes */                                                                                                         \
      int(sshbug_ignore1) int(sshbug_plainpw1) int(sshbug_rsa1) int(sshbug_hmac2) int(sshbug_derivekey2) int(                                                                                      \
          sshbug_rsapad2) int(sshbug_pksessid2) int(sshbug_rekey2) int(sshbug_maxpkt2) int(sshbug_ignore2) int(ssh_simple) /* Options for pterm. Should split out into platform-dependent part. */ \
      int(stamp_utmp) int(login_shell) int(scrollbar_on_left) int(shadowbold) FontSpec(boldfont)                                                                                                   \
          FontSpec(widefont) FontSpec(wideboldfont) int(shadowboldoffset) int(crhaslf)                                                                                                             \
              QUTTY_SERIALIZE_ELEMENT_ARRAY(char, winclass, 256)

#define int(a) int a;
#define Filename(a) Filename a;
#define FontSpec(a) FontSpec a;
#define QUTTY_SERIALIZE_ELEMENT_ARRAY(type, name, arr) type name[arr];
  QUTTY_SERIALIZE_STRUCT_CONFIG_ELEMENT_LIST
#undef QUTTY_SERIALIZE_ELEMENT_ARRAY
#undef int
#undef Filename
#undef FontSpec

  unsigned char colours[22][3];
  char *remote_cmd_ptr;  /* might point(to a larger command \
          * but never for loading/saving */
  char *remote_cmd_ptr2; /* might point(to a larger command \
          * but never for loading/saving */
};

#ifdef __cplusplus
}
#endif

#endif  // QTCONFIGTAG_H
