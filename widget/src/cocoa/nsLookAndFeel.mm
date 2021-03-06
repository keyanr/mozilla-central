/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Josh Aas <josh@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsLookAndFeel.h"
#include "nsObjCExceptions.h"
#include "nsIServiceManager.h"
#include "nsNativeThemeColors.h"
#include "nsStyleConsts.h"

#import <Cocoa/Cocoa.h>

nsLookAndFeel::nsLookAndFeel() : nsXPLookAndFeel()
{
}

nsLookAndFeel::~nsLookAndFeel()
{
}

static nscolor GetColorFromNSColor(NSColor* aColor)
{
  NSColor* deviceColor = [aColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
  return NS_RGB((unsigned int)([deviceColor redComponent] * 255.0),
                (unsigned int)([deviceColor greenComponent] * 255.0),
                (unsigned int)([deviceColor blueComponent] * 255.0));
}

nsresult
nsLookAndFeel::NativeGetColor(ColorID aID, nscolor &aColor)
{
  nsresult res = NS_OK;
  
  switch (aID) {
    case eColorID_WindowBackground:
      aColor = NS_RGB(0xff,0xff,0xff);
      break;
    case eColorID_WindowForeground:
      aColor = NS_RGB(0x00,0x00,0x00);        
      break;
    case eColorID_WidgetBackground:
      aColor = NS_RGB(0xdd,0xdd,0xdd);
      break;
    case eColorID_WidgetForeground:
      aColor = NS_RGB(0x00,0x00,0x00);        
      break;
    case eColorID_WidgetSelectBackground:
      aColor = NS_RGB(0x80,0x80,0x80);
      break;
    case eColorID_WidgetSelectForeground:
      aColor = NS_RGB(0x00,0x00,0x80);
      break;
    case eColorID_Widget3DHighlight:
      aColor = NS_RGB(0xa0,0xa0,0xa0);
      break;
    case eColorID_Widget3DShadow:
      aColor = NS_RGB(0x40,0x40,0x40);
      break;
    case eColorID_TextBackground:
      aColor = NS_RGB(0xff,0xff,0xff);
      break;
    case eColorID_TextForeground:
      aColor = NS_RGB(0x00,0x00,0x00);
      break;
    case eColorID_TextSelectBackground:
      aColor = GetColorFromNSColor([NSColor selectedTextBackgroundColor]);
      break;
    case eColorID_highlight: // CSS2 color
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlColor]);
      break;
    case eColorID__moz_menuhover:
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlColor]);
      break;      
    case eColorID_TextSelectForeground:
      GetColor(eColorID_TextSelectBackground, aColor);
      if (aColor == 0x000000)
        aColor = NS_RGB(0xff,0xff,0xff);
      else
        aColor = NS_DONT_CHANGE_COLOR;
      break;
    case eColorID_highlighttext:  // CSS2 color
    case eColorID__moz_menuhovertext:
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlTextColor]);
      break;
    case eColorID_IMESelectedRawTextBackground:
    case eColorID_IMESelectedConvertedTextBackground:
    case eColorID_IMERawInputBackground:
    case eColorID_IMEConvertedTextBackground:
      aColor = NS_TRANSPARENT;
      break;
    case eColorID_IMESelectedRawTextForeground:
    case eColorID_IMESelectedConvertedTextForeground:
    case eColorID_IMERawInputForeground:
    case eColorID_IMEConvertedTextForeground:
      aColor = NS_SAME_AS_FOREGROUND_COLOR;
      break;
    case eColorID_IMERawInputUnderline:
    case eColorID_IMEConvertedTextUnderline:
      aColor = NS_40PERCENT_FOREGROUND_COLOR;
      break;
    case eColorID_IMESelectedRawTextUnderline:
    case eColorID_IMESelectedConvertedTextUnderline:
      aColor = NS_SAME_AS_FOREGROUND_COLOR;
      break;
    case eColorID_SpellCheckerUnderline:
      aColor = NS_RGB(0xff, 0, 0);
      break;

      //
      // css2 system colors http://www.w3.org/TR/REC-CSS2/ui.html#system-colors
      //
      // It's really hard to effectively map these to the Appearance Manager properly,
      // since they are modeled word for word after the win32 system colors and don't have any 
      // real counterparts in the Mac world. I'm sure we'll be tweaking these for 
      // years to come. 
      //
      // Thanks to mpt26@student.canterbury.ac.nz for the hardcoded values that form the defaults
      //  if querying the Appearance Manager fails ;)
      //
      
    case eColorID_buttontext:
    case eColorID__moz_buttonhovertext:
      aColor = GetColorFromNSColor([NSColor controlTextColor]);
      break;
    case eColorID_captiontext:
    case eColorID_menutext:
    case eColorID_infotext:
    case eColorID__moz_menubartext:
      aColor = GetColorFromNSColor([NSColor textColor]);
      break;
    case eColorID_windowtext:
      aColor = GetColorFromNSColor([NSColor windowFrameTextColor]);
      break;
    case eColorID_activecaption:
      aColor = GetColorFromNSColor([NSColor gridColor]);
      break;
    case eColorID_activeborder:
      aColor = NS_RGB(0x00,0x00,0x00);
      break;
     case eColorID_appworkspace:
      aColor = NS_RGB(0xFF,0xFF,0xFF);
      break;
    case eColorID_background:
      aColor = NS_RGB(0x63,0x63,0xCE);
      break;
    case eColorID_buttonface:
    case eColorID__moz_buttonhoverface:
      aColor = NS_RGB(0xF0,0xF0,0xF0);
      break;
    case eColorID_buttonhighlight:
      aColor = NS_RGB(0xFF,0xFF,0xFF);
      break;
    case eColorID_buttonshadow:
      aColor = NS_RGB(0xDC,0xDC,0xDC);
      break;
    case eColorID_graytext:
      aColor = GetColorFromNSColor([NSColor disabledControlTextColor]);
      break;
    case eColorID_inactiveborder:
      aColor = GetColorFromNSColor([NSColor controlBackgroundColor]);
      break;
    case eColorID_inactivecaption:
      aColor = GetColorFromNSColor([NSColor controlBackgroundColor]);
      break;
    case eColorID_inactivecaptiontext:
      aColor = NS_RGB(0x45,0x45,0x45);
      break;
    case eColorID_scrollbar:
      aColor = GetColorFromNSColor([NSColor scrollBarColor]);
      break;
    case eColorID_threeddarkshadow:
      aColor = NS_RGB(0xDC,0xDC,0xDC);
      break;
    case eColorID_threedshadow:
      aColor = NS_RGB(0xE0,0xE0,0xE0);
      break;
    case eColorID_threedface:
      aColor = NS_RGB(0xF0,0xF0,0xF0);
      break;
    case eColorID_threedhighlight:
      aColor = GetColorFromNSColor([NSColor highlightColor]);
      break;
    case eColorID_threedlightshadow:
      aColor = NS_RGB(0xDA,0xDA,0xDA);
      break;
    case eColorID_menu:
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlTextColor]);
      break;
    case eColorID_infobackground:
      aColor = NS_RGB(0xFF,0xFF,0xC7);
      break;
    case eColorID_windowframe:
      aColor = GetColorFromNSColor([NSColor gridColor]);
      break;
    case eColorID_window:
    case eColorID__moz_field:
    case eColorID__moz_combobox:
      aColor = NS_RGB(0xff,0xff,0xff);
      break;
    case eColorID__moz_fieldtext:
    case eColorID__moz_comboboxtext:
      aColor = GetColorFromNSColor([NSColor controlTextColor]);
      break;
    case eColorID__moz_dialog:
      aColor = GetColorFromNSColor([NSColor controlHighlightColor]);
      break;
    case eColorID__moz_dialogtext:
    case eColorID__moz_cellhighlighttext:
    case eColorID__moz_html_cellhighlighttext:
      aColor = GetColorFromNSColor([NSColor controlTextColor]);
      break;
    case eColorID__moz_dragtargetzone:
      aColor = GetColorFromNSColor([NSColor selectedControlColor]);
      break;
    case eColorID__moz_mac_chrome_active:
    case eColorID__moz_mac_chrome_inactive: {
      int grey = NativeGreyColorAsInt(toolbarFillGrey, (aID == eColorID__moz_mac_chrome_active));
      aColor = NS_RGB(grey, grey, grey);
    }
      break;
    case eColorID__moz_mac_focusring:
      aColor = GetColorFromNSColor([NSColor keyboardFocusIndicatorColor]);
      break;
    case eColorID__moz_mac_menushadow:
      aColor = NS_RGB(0xA3,0xA3,0xA3);
      break;          
    case eColorID__moz_mac_menutextdisable:
      aColor = nsToolkit::OnSnowLeopardOrLater() ?
                 NS_RGB(0x88,0x88,0x88) : NS_RGB(0x98,0x98,0x98);
      break;      
    case eColorID__moz_mac_menutextselect:
      aColor = GetColorFromNSColor([NSColor selectedMenuItemTextColor]);
      break;      
    case eColorID__moz_mac_disabledtoolbartext:
      aColor = NS_RGB(0x3F,0x3F,0x3F);
      break;
    case eColorID__moz_mac_menuselect:
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlColor]);
      break;
    case eColorID__moz_buttondefault:
      aColor = NS_RGB(0xDC,0xDC,0xDC);
      break;
    case eColorID__moz_mac_alternateprimaryhighlight:
      aColor = GetColorFromNSColor([NSColor alternateSelectedControlColor]);
      break;
    case eColorID__moz_cellhighlight:
    case eColorID__moz_html_cellhighlight:
    case eColorID__moz_mac_secondaryhighlight:
      // For inactive list selection
      aColor = GetColorFromNSColor([NSColor secondarySelectedControlColor]);
      break;
    case eColorID__moz_eventreerow:
      // Background color of even list rows.
      aColor = GetColorFromNSColor([[NSColor controlAlternatingRowBackgroundColors] objectAtIndex:0]);
      break;
    case eColorID__moz_oddtreerow:
      // Background color of odd list rows.
      aColor = GetColorFromNSColor([[NSColor controlAlternatingRowBackgroundColors] objectAtIndex:1]);
      break;
    case eColorID__moz_nativehyperlinktext:
      // There appears to be no available system defined color. HARDCODING to the appropriate color.
      aColor = NS_RGB(0x14,0x4F,0xAE);
      break;
    default:
      NS_WARNING("Someone asked nsILookAndFeel for a color I don't know about");
      aColor = NS_RGB(0xff,0xff,0xff);
      res = NS_ERROR_FAILURE;
      break;
    }
  
  return res;
}

nsresult
nsLookAndFeel::GetIntImpl(IntID aID, PRInt32 &aResult)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  nsresult res = nsXPLookAndFeel::GetIntImpl(aID, aResult);
  if (NS_SUCCEEDED(res))
    return res;
  res = NS_OK;
  
  switch (aID) {
    case eIntID_CaretBlinkTime:
      aResult = 567;
      break;
    case eIntID_CaretWidth:
      aResult = 1;
      break;
    case eIntID_ShowCaretDuringSelection:
      aResult = 0;
      break;
    case eIntID_SelectTextfieldsOnKeyFocus:
      // Select textfield content when focused by kbd
      // used by nsEventStateManager::sTextfieldSelectModel
      aResult = 1;
      break;
    case eIntID_SubmenuDelay:
      aResult = 200;
      break;
    case eIntID_MenusCanOverlapOSBar:
      // xul popups are not allowed to overlap the menubar.
      aResult = 0;
      break;
    case eIntID_SkipNavigatingDisabledMenuItem:
      aResult = 1;
      break;
    case eIntID_DragThresholdX:
    case eIntID_DragThresholdY:
      aResult = 4;
      break;
    case eIntID_ScrollArrowStyle:
      if (nsToolkit::OnLionOrLater()) {
        // OS X Lion's scrollbars have no arrows
        aResult = eScrollArrow_None;
      } else {
        NSString *buttonPlacement = [[NSUserDefaults standardUserDefaults] objectForKey:@"AppleScrollBarVariant"];
        if ([buttonPlacement isEqualToString:@"Single"]) {
          aResult = eScrollArrowStyle_Single;
        } else if ([buttonPlacement isEqualToString:@"DoubleMin"]) {
          aResult = eScrollArrowStyle_BothAtTop;
        } else if ([buttonPlacement isEqualToString:@"DoubleBoth"]) {
          aResult = eScrollArrowStyle_BothAtEachEnd;
        } else {
          aResult = eScrollArrowStyle_BothAtBottom; // The default is BothAtBottom.
        }
      }
      break;
    case eIntID_ScrollSliderStyle:
      aResult = eScrollThumbStyle_Proportional;
      break;
    case eIntID_TreeOpenDelay:
      aResult = 1000;
      break;
    case eIntID_TreeCloseDelay:
      aResult = 1000;
      break;
    case eIntID_TreeLazyScrollDelay:
      aResult = 150;
      break;
    case eIntID_TreeScrollDelay:
      aResult = 100;
      break;
    case eIntID_TreeScrollLinesMax:
      aResult = 3;
      break;
    case eIntID_DWMCompositor:
    case eIntID_WindowsClassic:
    case eIntID_WindowsDefaultTheme:
    case eIntID_TouchEnabled:
    case eIntID_MaemoClassic:
    case eIntID_WindowsThemeIdentifier:
      aResult = 0;
      res = NS_ERROR_NOT_IMPLEMENTED;
      break;
    case eIntID_MacGraphiteTheme:
      aResult = [NSColor currentControlTint] == NSGraphiteControlTint;
      break;
    case eIntID_MacLionTheme:
      aResult = nsToolkit::OnLionOrLater();
      break;
    case eIntID_TabFocusModel:
    {
      // we should probably cache this
      CFPropertyListRef fullKeyboardAccessProperty;
      fullKeyboardAccessProperty = ::CFPreferencesCopyValue(CFSTR("AppleKeyboardUIMode"),
                                                            kCFPreferencesAnyApplication,
                                                            kCFPreferencesCurrentUser,
                                                            kCFPreferencesAnyHost);
      aResult = 1;    // default to just textboxes
      if (fullKeyboardAccessProperty) {
        PRInt32 fullKeyboardAccessPrefVal;
        if (::CFNumberGetValue((CFNumberRef) fullKeyboardAccessProperty, kCFNumberIntType, &fullKeyboardAccessPrefVal)) {
          // the second bit means  "Full keyboard access" is on
          if (fullKeyboardAccessPrefVal & (1 << 1))
            aResult = 7; // everything that can be focused
        }
        ::CFRelease(fullKeyboardAccessProperty);
      }
    }
      break;
    case eIntID_ScrollToClick:
    {
      aResult = [[NSUserDefaults standardUserDefaults] boolForKey:@"AppleScrollerPagingBehavior"];
    }
      break;
    case eIntID_ChosenMenuItemsShouldBlink:
      aResult = 1;
      break;
    case eIntID_IMERawInputUnderlineStyle:
    case eIntID_IMEConvertedTextUnderlineStyle:
    case eIntID_IMESelectedRawTextUnderlineStyle:
    case eIntID_IMESelectedConvertedTextUnderline:
      aResult = NS_STYLE_TEXT_DECORATION_STYLE_SOLID;
      break;
    case eIntID_SpellCheckerUnderlineStyle:
      aResult = NS_STYLE_TEXT_DECORATION_STYLE_DOTTED;
      break;
    case eIntID_ScrollbarButtonAutoRepeatBehavior:
      aResult = 0;
      break;
    default:
      aResult = 0;
      res = NS_ERROR_FAILURE;
  }
  return res;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}

nsresult
nsLookAndFeel::GetFloatImpl(FloatID aID, float &aResult)
{
  nsresult res = nsXPLookAndFeel::GetFloatImpl(aID, aResult);
  if (NS_SUCCEEDED(res))
    return res;
  res = NS_OK;
  
  switch (aID) {
    case eFloatID_IMEUnderlineRelativeSize:
      aResult = 2.0f;
      break;
    case eFloatID_SpellCheckerUnderlineRelativeSize:
      aResult = 2.0f;
      break;
    default:
      aResult = -1.0;
      res = NS_ERROR_FAILURE;
  }

  return res;
}
