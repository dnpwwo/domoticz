#pragma once

#include <string>
#include "../webserver/cWebem.h"
#include "../webserver/request.hpp"
#include "../webserver/session_store.hpp"

struct lua_State;
struct lua_Debug;

namespace Json
{
	class Value;
};

namespace http {
	namespace server {
		class cWebem;
		struct _tWebUserPassword;
class CWebServer : public session_store, public std::enable_shared_from_this<CWebServer>
{
	typedef boost::function< void(WebEmSession & session, const request& req, Json::Value &root) > webserver_response_function;
public:
	struct _tCustomIcon
	{
		int idx;
		std::string RootFile;
		std::string Title;
		std::string Description;
	};
	CWebServer(void);
	~CWebServer(void);
	bool StartServer(server_settings & settings, const std::string &serverpath, const bool bIgnoreUsernamePassword);
	void StopServer();
	void RegisterCommandCode(const char* idname, webserver_response_function ResponseFunction, bool bypassAuthentication=false);
	void RegisterRType(const char* idname, webserver_response_function ResponseFunction);

	void DisplaySwitchTypesCombo(std::string & content_part);
	void DisplayMeterTypesCombo(std::string & content_part);
	void DisplayTimerTypesCombo(std::string & content_part);
	void DisplayLanguageCombo(std::string & content_part);
	void GetJSonPage(WebEmSession & session, const request& req, reply & rep);
	void GetAppCache(WebEmSession & session, const request& req, reply & rep);
	void GetCameraSnapshot(WebEmSession & session, const request& req, reply & rep);
	void GetInternalCameraSnapshot(WebEmSession & session, const request& req, reply & rep);
	void GetFloorplanImage(WebEmSession & session, const request& req, reply & rep);
	void GetDatabaseBackup(WebEmSession & session, const request& req, reply & rep);
	void Post_UploadCustomIcon(WebEmSession & session, const request& req, reply & rep);

	void UploadFloorplanImage(WebEmSession & session, const request& req, std::string & redirect_uri);
	void RestoreDatabase(WebEmSession & session, const request& req, std::string & redirect_uri);

	void EventCreate(WebEmSession & session, const request& req, std::string & redirect_uri);

	cWebem *m_pWebEm;

	void ReloadCustomSwitchIcons();

	void LoadUsers();
	void AddUser(const unsigned long ID, const std::string &username, const std::string &password, const int userrights, const int activetabs);
	void ClearUserPasswords();
	bool FindAdminUser();
	int FindUser(const char* szUserName);
	void SetWebCompressionMode(const _eWebCompressionMode gzmode);
	void SetAuthenticationMethod(const _eAuthenticationMethod amethod);
	void SetWebTheme(const std::string &themename);
	void SetWebRoot(const std::string &webRoot);
	std::vector<_tWebUserPassword> m_users;

	// SessionStore interface
	const WebEmStoredSession GetSession(const std::string & sessionId) override;
	void StoreSession(const WebEmStoredSession & session) override;
	void RemoveSession(const std::string & sessionId) override;
	void CleanSessions() override;
	void RemoveUsersSessions(const std::string& username, const WebEmSession & exceptSession);
	std::string PluginHardwareDesc(int HwdID);

private:
	void HandleCommand(const std::string &cparam, WebEmSession & session, const request& req, Json::Value &root);
	void HandleRType(const std::string &rtype, WebEmSession & session, const request& req, Json::Value &root);

	bool IsIdxForUser(const WebEmSession *pSession, const int Idx);

	//Commands
	void Cmd_RFXComGetFirmwarePercentage(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetLanguage(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetThemes(WebEmSession & session, const request& req, Json::Value &root);
        void Cmd_GetTitle(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_LoginCheck(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetHardwareTypes(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddHardware(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateHardware(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteHardware(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetDeviceValueOptions(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetDeviceValueOptionWording(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AllowNewHardware(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetLog(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ClearLog(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddPlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdatePlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeletePlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetUnusedPlanDevices(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddPlanActiveDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetPlanDevices(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeletePlanDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SetPlanDeviceCoords(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteAllPlanDevices(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ChangePlanOrder(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ChangePlanDeviceOrder(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetVersion(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetAuth(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetUptime(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetActualHistory(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetNewHistory(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetConfig(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SendNotification(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateDevices(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SystemShutdown(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SystemReboot(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ExcecuteScript(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetCosts(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_CheckForUpdate(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DownloadUpdate(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DownloadReady(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteDatePoint(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SetActiveTimerPlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_EnableTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DisableTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ClearTimers(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddSceneTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateSceneTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteSceneTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_EnableSceneTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DisableSceneTimer(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ClearSceneTimers(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetSceneActivations(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddSceneCode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_RemoveSceneCode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ClearSceneCodes(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_RenameScene(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetSerialDevices(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetDevicesList(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetDevicesListOnOff(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_GetCustomIconSet(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteCustomIcon(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateCustomIcon(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_RenameDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_SetUnused(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddLogMessage(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ClearShortLog(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_VacuumDatabase(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddMobileDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateMobileDevice(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteMobileDevice(WebEmSession & session, const request& req, Json::Value &root);

	void Cmd_GetTimerPlans(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_AddTimerPlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateTimerPlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteTimerPlan(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DuplicateTimerPlan(WebEmSession & session, const request& req, Json::Value &root);

	void Cmd_AddCamera(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_UpdateCamera(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_DeleteCamera(WebEmSession & session, const request& req, Json::Value &root);

	// Plugin functions
	void Cmd_PluginCommand(WebEmSession & session, const request& req, Json::Value &root);
	void PluginList(Json::Value &root);
#ifdef ENABLE_PYTHON
	void PluginLoadConfig();
#endif

	//RTypes
	void RType_LightLog(WebEmSession & session, const request& req, Json::Value &root);
	void RType_TextLog(WebEmSession & session, const request& req, Json::Value &root);
	void RType_SceneLog(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Settings(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Events(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Hardware(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Devices(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Cameras(WebEmSession& session, const request& req, Json::Value& root);
	void RType_CamerasUser(WebEmSession& session, const request& req, Json::Value& root);
	void RType_Users(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Mobiles(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Timers(WebEmSession & session, const request& req, Json::Value &root);
	void RType_SceneTimers(WebEmSession & session, const request& req, Json::Value &root);
	void RType_SetpointTimers(WebEmSession & session, const request& req, Json::Value &root);
	void RType_GetTransfers(WebEmSession & session, const request& req, Json::Value &root);
	void RType_TransferDevice(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Notifications(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Schedules(WebEmSession & session, const request& req, Json::Value &root);
	void RType_GetSharedUserDevices(WebEmSession & session, const request& req, Json::Value &root);
	void RType_SetSharedUserDevices(WebEmSession & session, const request& req, Json::Value &root);
	void RType_SetUsed(WebEmSession & session, const request& req, Json::Value &root);
	void RType_DeleteDevice(WebEmSession & session, const request& req, Json::Value &root);
	void RType_AddScene(WebEmSession & session, const request& req, Json::Value &root);
	void RType_DeleteScene(WebEmSession & session, const request& req, Json::Value &root);
	void RType_UpdateScene(WebEmSession & session, const request& req, Json::Value &root);
	void RType_CustomLightIcons(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Plans(WebEmSession & session, const request& req, Json::Value &root);
	void RType_FloorPlans(WebEmSession & session, const request& req, Json::Value &root);
	void RType_Scenes(WebEmSession & session, const request& req, Json::Value &root);
#ifdef WITH_OPENZWAVE
	//ZWave
	void Cmd_ZWaveUpdateNode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveDeleteNode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveInclude(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveExclude(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveIsNodeIncluded(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveIsNodeExcluded(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveSoftReset(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveHardReset(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveNetworkHeal(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveNodeHeal(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveNetworkInfo(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveRemoveGroupNode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveAddGroupNode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveGroupInfo(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveCancel(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ApplyZWaveNodeConfig(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveStateCheck(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveRequestNodeConfig(WebEmSession& session, const request& req, Json::Value& root);
	void Cmd_ZWaveRequestNodeInfo(WebEmSession& session, const request& req, Json::Value& root);
	void Cmd_ZWaveReceiveConfigurationFromOtherController(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveSendConfigurationToSecondaryController(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveTransferPrimaryRole(WebEmSession & session, const request& req, Json::Value &root);
	void ZWaveGetConfigFile(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPPollXml(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPIndex(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPNodeGetConf(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPNodeGetValues(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPNodeSetValue(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPNodeSetButton(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPAdminCommand(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPNodeChange(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPSaveConfig(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPGetTopo(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPGetStats(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPSetGroup(WebEmSession & session, const request& req, reply & rep);
	void ZWaveCPSceneCommand(WebEmSession & session, const request& req, reply & rep);
	void Cmd_ZWaveSetUserCodeEnrollmentMode(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveGetNodeUserCodes(WebEmSession & session, const request& req, Json::Value &root);
	void Cmd_ZWaveRemoveUserCode(WebEmSession & session, const request& req, Json::Value &root);
	void ZWaveCPTestHeal(WebEmSession & session, const request& req, reply & rep);
	void Cmd_ZWaveGetBatteryLevels(WebEmSession& session, const request& req, Json::Value& root);
	//RTypes
	void RType_OpenZWaveNodes(WebEmSession & session, const request& req, Json::Value &root);
	int m_ZW_Hwidx;
#endif
    void Cmd_TellstickApplySettings(WebEmSession &session, const request &req, Json::Value &root);
	std::shared_ptr<std::thread> m_thread;

	std::map < std::string, webserver_response_function > m_webcommands;
	std::map < std::string, webserver_response_function > m_webrtypes;
	void Do_Work();
	std::vector<_tCustomIcon> m_custom_light_icons;
	std::map<int, int> m_custom_light_icons_lookup;
	bool m_bDoStop;
	std::string m_server_alias;
};

} //server
}//http
