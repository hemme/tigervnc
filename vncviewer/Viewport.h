/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011-2021 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include <core/Rect.h>

#include <FL/Fl_Widget.H>

#include "EmulateMB.h"
#include "Keyboard.h"
#include "ShortcutHandler.h"

#include <vector>
#include <set>
#include <map>
#include <string>

class Fl_Menu_Button;
class Fl_RGB_Image;

class CConn;
class Keyboard;
class PlatformPixelBuffer;
class SendTextDialog;
class Surface;

class Viewport : public Fl_Widget, protected EmulateMB,
                 protected KeyboardHandler {
public:

  Viewport(int w, int h, CConn* cc_);
  ~Viewport();

  void popupContextMenu();
  void executeMenuAction(int id, bool toggleValue = false);
#ifdef WIN32
  void popupNativeContextMenu();
#endif

  // Most efficient format (from Viewport's point of view)
  const rfb::PixelFormat &getPreferredPF();

  // Native (server) framebuffer dimensions, which may differ from the
  // widget size when the desktop is scaled to fit the window.
  int framebufferWidth() const;
  int framebufferHeight() const;

  // Recreate the framebuffer to the given (native) dimensions and
  // resize the widget to the scaled visual size.
  void resizeFramebuffer(int w, int h);

  // Visual scale factor applied when rendering (1.0 == unscaled).
  void setScale(double s);
  double getScale() const { return scale; }

  // Flush updates to screen
  void updateWindow();

  // New image for the locally rendered cursor
  void setCursor();

  // Change client LED state
  void setLEDState(unsigned int state);

  void draw(Surface* dst);

  // Clipboard events
  void handleClipboardRequest();
  void handleClipboardAnnounce(bool available);
  void handleClipboardData(const char* data);

  // Fl_Widget callback methods

  void draw() override;

  void resize(int x, int y, int w, int h) override;

  int handle(int event) override;

protected:
  void sendPointerEvent(const core::Point& pos,
                        uint16_t buttonMask) override;

private:
  bool hasFocus();

  // Convert a widget-local (scaled) coordinate to native framebuffer
  // coordinates, taking the current scale factor into account.
  core::Point mapToFramebuffer(const core::Point& p) const;

  // Show the currently set (or system) cursor
  void showCursor();

  static void handleClipboardChange(int source, void *data);

  void flushPendingClipboard();

  void handlePointerEvent(const core::Point& pos, uint16_t buttonMask);
  static void handlePointerTimeout(void *data);

  void resetKeyboard();

  void sendTextStrokes(const std::string& text);
  void stopTextStrokes();
  static void sendTextStrokesTimeout(void *data);

  void showSendTextDialog();
  static void sendTextFromDialog(const std::string& text, void *data);

  void handleKeyPress(int systemKeyCode,
                      uint32_t keyCode, uint32_t keySym) override;
  void sendKeyPress(int systemKeyCode,
                    uint32_t keyCode, uint32_t keySym);
  void handleKeyRelease(int systemKeyCode) override;
  void sendKeyRelease(int systemKeyCode);
  void sendKeyRepeat(int systemKeyCode);

  static int handleSystemEvent(void *event, void *data);

  void pushLEDState();

  void initContextMenu();

  static void popupContextMenuTimeout(void *data);

  static void handleOptions(void *data);

private:
  CConn* cc;

  PlatformPixelBuffer* frameBuffer;

  double scale;

  core::Point lastPointerPos;
  uint16_t lastButtonMask;

  Keyboard* keyboard;
  ShortcutHandler shortcutHandler;
  bool shortcutBypass;
  bool shortcutActive;
  std::set<int> pressedKeys;

  bool firstLEDState;

  bool pendingClientClipboard;

  int clipboardSource;

  Fl_Menu_Button *contextMenu;

  SendTextDialog *sendTextDialog;

  bool menuCtrlKey;
  bool menuAltKey;

  bool strokeSending;
  std::string strokeQueue;
  size_t strokePos;

  Fl_RGB_Image *cursor;
  core::Point cursorHotspot;
  bool cursorIsBlank;

  struct KeyMapping {
    std::set<uint32_t> sourceKeys;
    std::vector<uint32_t> targetKeys;
  };
  std::vector<KeyMapping> keyMappingsList;
  std::set<uint32_t> physicalPressedKeysyms;
  std::map<int, uint32_t> physicalKeyToKeysym;
  std::set<uint32_t> serverPressedKeysyms;
  std::map<uint32_t, int> keysymToSentSystemKeyCode;

  // Client-side key repeat: toggled once per repeated key event so that
  // only every other system repeat is forwarded to the host (half the
  // local repeat rate)
  std::map<int, bool> keyRepeatParity;

  // Physical system key code -> server keysyms it is currently sent as,
  // and the system key code actually used for each of those keysyms
  // (needed to repeat keys sent under a synthetic code because of key
  // mappings)
  std::map<int, std::map<uint32_t, int>> physicalKeyToSentKeys;

  struct PhysicalKeyEvent {
    int systemKeyCode;
    uint32_t keyCode;
  };
  std::map<uint32_t, PhysicalKeyEvent> keysymToPhysicalEvent;

  void parseKeyMappings();
  void updateKeyMappingState();
};

#endif
