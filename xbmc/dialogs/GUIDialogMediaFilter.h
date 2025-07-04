/*
 *  Copyright (C) 2012-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "dbwrappers/Database.h"
#include "dbwrappers/DatabaseQuery.h"
#include "settings/dialogs/GUIDialogSettingsManualBase.h"
#include "settings/lib/SettingType.h"
#include "utils/DatabaseUtils.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CDbUrl;
class CSetting;
namespace KODI::PLAYLIST
{
class CSmartPlaylist;
class CSmartPlaylistRule;
} // namespace KODI::PLAYLIST
struct StringSettingOption;

class CGUIDialogMediaFilter : public CGUIDialogSettingsManualBase
{
public:
  CGUIDialogMediaFilter();
  ~CGUIDialogMediaFilter() override;

  // specializations of CGUIControl
  bool OnMessage(CGUIMessage &message) override;

  static void ShowAndEditMediaFilter(const std::string& path,
                                     KODI::PLAYLIST::CSmartPlaylist& filter);

  struct Filter
  {
    std::string mediaType;
    Field field;
    uint32_t label;
    SettingType settingType;
    std::string controlType;
    std::string controlFormat;
    CDatabaseQueryRule::SearchOperator ruleOperator;
    std::shared_ptr<CSetting> setting = nullptr;
    KODI::PLAYLIST::CSmartPlaylistRule* rule = nullptr;
    void* data = nullptr;
  };

protected:
  // specializations of CGUIWindow
  void OnWindowLoaded() override;
  void OnInitWindow() override;

  // implementations of ISettingCallback
  void OnSettingChanged(const std::shared_ptr<const CSetting>& setting) override;

  // specialization of CGUIDialogSettingsBase
  bool AllowResettingSettings() const override { return false; }
  bool Save() override { return true; }
  std::chrono::milliseconds GetDelayMs() const override { return std::chrono::milliseconds(500); }

  // specialization of CGUIDialogSettingsManualBase
  void SetupView() override;
  void InitializeSettings() override;

  bool SetPath(const std::string &path);
  void UpdateControls();
  void TriggerFilter() const;
  void Reset(bool filtersOnly = false);

  int GetItems(const Filter &filter, std::vector<std::string> &items, bool countOnly = false);
  void GetRange(const Filter &filter, int &min, int &interval, int &max);
  void GetRange(const Filter &filter, float &min, float &interval, float &max);
  bool GetMinMax(const std::string &table, const std::string &field, int &min, int &max, const CDatabase::Filter &filter = CDatabase::Filter());

  KODI::PLAYLIST::CSmartPlaylistRule* AddRule(
      Field field,
      CDatabaseQueryRule::SearchOperator ruleOperator =
          CDatabaseQueryRule::SearchOperator::OPERATOR_CONTAINS);
  void DeleteRule(Field field);

  void GetStringListOptions(const std::shared_ptr<const CSetting>& setting,
                            std::vector<StringSettingOption>& list,
                            std::string& current);

  CDbUrl* m_dbUrl;
  std::string m_mediaType;
  KODI::PLAYLIST::CSmartPlaylist* m_filter;
  std::map<std::string, Filter> m_filters;
};
