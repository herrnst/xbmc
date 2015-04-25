#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDInputStream.h"
#include <list>
#include <memory>

extern "C"
{
#include <libbluray/bluray.h>
#include <libbluray/bluray-version.h>
#include <libbluray/keys.h>
#include <libbluray/overlay.h>
#include <libbluray/player_settings.h>
#include <libavcodec/avcodec.h>
}

#define MAX_PLAYLIST_ID 99999
#define BD_EVENT_MENU_OVERLAY -1

class CDVDOverlayImage;
class DllLibbluray;
class IDVDPlayer;

typedef struct BDNavMessage
{
  bool useraction;
  void* data;

  BDNavMessage() :
    useraction(false),
    data(NULL){};
} BDNavMessage;

class CDVDInputStreamBluray 
  : public CDVDInputStream
  , public CDVDInputStream::IDisplayTime
  , public CDVDInputStream::IChapter
  , public CDVDInputStream::ISeekTime
  , public CDVDInputStream::IMenus
{
public:
  CDVDInputStreamBluray(IDVDPlayer* player);
  virtual ~CDVDInputStreamBluray();
  virtual bool Open(const char* strFile, const std::string &content, bool contentLookup);
  virtual void Close();
  virtual int Read(uint8_t* buf, int buf_size);
  virtual int64_t Seek(int64_t offset, int whence);
  virtual bool Pause(double dTime) { return false; };
  void Abort();
  virtual bool IsEOF();
  virtual int64_t GetLength();
  virtual int GetBlockSize() { return 6144; }
  virtual ENextStream NextStream();


  /* IMenus */
  virtual void ActivateButton()          { UserInput(BD_VK_ENTER, -1); }
  virtual void SelectButton(int iButton)
  {
    if(iButton < 10)
      UserInput((bd_vk_key_e)(BD_VK_0 + iButton), -1);
  }
  virtual int  GetCurrentButton()        { return 0; }
  virtual int  GetTotalButtons()         { return 0; }
  virtual void OnUp(int64_t pts)                    { UserInput(BD_VK_UP, pts); }
  virtual void OnDown(int64_t pts)                  { UserInput(BD_VK_DOWN, pts); }
  virtual void OnLeft(int64_t pts)                  { UserInput(BD_VK_LEFT, pts); }
  virtual void OnRight(int64_t pts)                 { UserInput(BD_VK_RIGHT, pts); }
  virtual void OnMenu(int64_t pts);
  virtual void OnBack(int64_t pts)
  {
    if(IsInMenu())
      OnMenu(pts);
  }
  virtual void OnNext(int64_t pts)                  {}
  virtual void OnPrevious(int64_t pts)              {}
  virtual bool HasMenu();
  virtual bool IsInMenu();
  virtual bool OnMouseMove(const CPoint &point)  { return MouseMove(point); }
  virtual bool OnMouseClick(const CPoint &point) { return MouseClick(point); }
  virtual double GetTimeStampCorrection()        { return 0.0; }
  virtual void SkipStill();
  virtual bool GetState(std::string &xmlstate)         { return false; }
  virtual bool SetState(const std::string &xmlstate)   { return false; }


  void UserInput(bd_vk_key_e vk, int64_t pts);
  bool MouseMove(const CPoint &point);
  bool MouseClick(const CPoint &point);

  int GetChapter();
  int GetChapterCount();
  void GetChapterName(std::string& name, int ch=-1) {};
  int64_t GetChapterPos(int ch);
  bool SeekChapter(int ch);

  int GetTotalTime();
  int GetTime();
  bool SeekTime(int ms);

  void GetStreamInfo(int pid, char* language);

  void OverlayCallback(const BD_OVERLAY * const);
#ifdef HAVE_LIBBLURAY_BDJ
  void OverlayCallbackARGB(const struct bd_argb_overlay_s * const);
#endif

  BLURAY_TITLE_INFO* GetTitleLongest();
  BLURAY_TITLE_INFO* GetTitleFile(const std::string& name);

  void ProcessEvent();

protected:
  struct SPlane;

  void OverlayFlush(int64_t pts);
  void OverlayClose();
  static void OverlayClear(SPlane& plane, int x, int y, int w, int h);
  static void OverlayInit (SPlane& plane, int w, int h);

  IDVDPlayer*         m_player;
  DllLibbluray*       m_dll;
  BLURAY*             m_bd;
  BLURAY_TITLE_INFO*  m_title;
  uint32_t            m_playlist;
  uint32_t            m_clip;
  uint32_t            m_angle;
  bool                m_menu;
  bool                m_navmode;
  uint32_t            m_useractionstart;

  typedef std::shared_ptr<CDVDOverlayImage> SOverlay;
  typedef std::list<SOverlay>                 SOverlays;

  struct SPlane
  {
    SOverlays o;
    int       w;
    int       h;

    SPlane()
    : w(0)
    , h(0)
    {}
  };

  SPlane m_planes[2];
  enum EHoldState {
    HOLD_NONE = 0,
    HOLD_HELD,
    HOLD_DATA,
    HOLD_STILL,
    HOLD_ERROR,
    HOLD_EXIT,
    HOLD_PAUSE
  } m_hold;
  BD_EVENT m_event;
#ifdef HAVE_LIBBLURAY_BDJ
  struct bd_argb_buffer_s m_argb;
#endif

  private:
    void SetupPlayerSettings();
};

typedef std::map<int, AVCodecID> BD_CODEC_MAP;
typedef std::map<int, AVCodecID>::const_iterator BD_CODEC_MAP_IT;

static const BD_CODEC_MAP bd_codec_map = {
  { 0x01, AV_CODEC_ID_MPEG1VIDEO },
  { 0x02, AV_CODEC_ID_MPEG2VIDEO },
  { 0x03, AV_CODEC_ID_MP1 },
  { 0x04, AV_CODEC_ID_MP2 },
  { 0x80, AV_CODEC_ID_PCM_BLURAY },
  { 0x81, AV_CODEC_ID_AC3 },
  { 0x82, AV_CODEC_ID_DTS },
  { 0x83, AV_CODEC_ID_TRUEHD },
  { 0x84, AV_CODEC_ID_EAC3 },
  { 0x85, AV_CODEC_ID_DTS }, //DTS-HD
  { 0x86, AV_CODEC_ID_DTS }, //DTS-HD Master
  { 0xa1, AV_CODEC_ID_NONE }, //AC-3 Plus for secondary audio
  { 0xa2, AV_CODEC_ID_NONE }, //DTS-HD for secondary audio
  { 0xea, AV_CODEC_ID_VC1 },
  { 0x1b, AV_CODEC_ID_H264 },
  { 0x20, AV_CODEC_ID_NONE }, //H.264 MVC dep.
  { 0x90, AV_CODEC_ID_NONE }, //Presentation Graphics
  { 0x91, AV_CODEC_ID_NONE }, //Presentation Graphics
  { 0x92, AV_CODEC_ID_NONE } //Interactive Graphics
};
