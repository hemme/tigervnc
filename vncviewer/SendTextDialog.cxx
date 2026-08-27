/* Copyright 2026 Marco Mariotti for Cendio AB
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_ask.H>

#include <core/i18n.h>

#include "fltk/layout.h"

#include "SendTextDialog.h"

SendTextDialog::SendTextDialog(SendTextCallback* sendCallback_,
                               void* sendData_)
  : Fl_Window(440, 300, _("Send text")), sendCallback(sendCallback_),
    sendData(sendData_)
{
  int x, y;

  Fl_Button* button;

  x = OUTER_MARGIN;
  y = OUTER_MARGIN;

  textInput = new Fl_Multiline_Input(x, y,
                                     w() - OUTER_MARGIN - x,
                                     h() - BUTTON_HEIGHT - INNER_MARGIN - OUTER_MARGIN - y);
  textInput->align(FL_ALIGN_TOP_LEFT);
  textInput->wrap(1);

  x = w() - OUTER_MARGIN;
  y = h() - BUTTON_HEIGHT - OUTER_MARGIN;

  x -= BUTTON_WIDTH;
  button = new Fl_Return_Button(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, _("Send"));
  button->callback(button_cb, 1);
  x -= INNER_MARGIN;

  x -= BUTTON_WIDTH;
  button = new Fl_Button(x, y, BUTTON_WIDTH, BUTTON_HEIGHT, fl_cancel);
  button->callback(button_cb, 0);
  button->shortcut(FL_Escape);

  end();

  // Non-modal so the user can still interact with the VNC session
  // (e.g. to focus the window that should receive the text)
  set_non_modal();
}

SendTextDialog::~SendTextDialog()
{
}

std::string SendTextDialog::getText()
{
  return textInput->value();
}

void SendTextDialog::button_cb(Fl_Widget *w, long val)
{
  SendTextDialog* self;

  self = dynamic_cast<SendTextDialog*>(w->window());
  assert(self != nullptr);

  self->hide();

  if ((val == 1) && self->sendCallback)
    self->sendCallback(self->getText(), self->sendData);
}
