		void CWebServer::GetJSonDevices(
			Json::Value &root,
			const std::string &rused,
			const std::string &rfilter,
			const std::string &order,
			const std::string &rowid,
			const std::string &planID,
			const std::string &floorID,
			const bool bDisplayHidden,
			const bool bDisplayDisabled,
			const bool bFetchFavorites,
			const time_t LastUpdate,
			const std::string &username,
			const std::string &hardwareid)
		{
			std::vector<std::vector<std::string> > result;

			time_t now = mytime(NULL);
			struct tm tm1;
			localtime_r(&now, &tm1);
			struct tm tLastUpdate;
			localtime_r(&now, &tLastUpdate);

			const time_t iLastUpdate = LastUpdate - 1;

			int SensorTimeOut = 60;
			m_sql.GetPreferencesVar("SensorTimeout", SensorTimeOut);

			std::map<int, _tHardwareListInt> _hardwareNames;
			result = m_sql.safe_query("SELECT ID, Name, Enabled, Type, Mode1, Mode2 FROM Hardware");
			if (!result.empty())
			{
				int ii = 0;
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;
					_tHardwareListInt tlist;
					int ID = atoi(sd[0].c_str());
					tlist.Name = sd[1];
					tlist.Enabled = (atoi(sd[2].c_str()) != 0);
					tlist.HardwareTypeVal = atoi(sd[3].c_str());
#ifndef ENABLE_PYTHON
					tlist.HardwareType = Hardware_Type_Desc(tlist.HardwareTypeVal);
#else
					if (tlist.HardwareTypeVal != HTYPE_PythonPlugin)
					{
						tlist.HardwareType = Hardware_Type_Desc(tlist.HardwareTypeVal);
					}
					else
					{
						tlist.HardwareType = PluginHardwareDesc(ID);
					}
#endif
					tlist.Mode1 = sd[4];
					tlist.Mode2 = sd[5];
					_hardwareNames[ID] = tlist;
				}
			}

			root["ActTime"] = static_cast<int>(now);

			char szTmp[300];

			if (!m_mainworker.m_LastSunriseSet.empty())
			{
				std::vector<std::string> strarray;
				StringSplit(m_mainworker.m_LastSunriseSet, ";", strarray);
				if (strarray.size() == 10)
				{
					strftime(szTmp, 80, "%Y-%m-%d %X", &tm1);
					root["ServerTime"] = szTmp;
					root["Sunrise"] = strarray[0];
					root["Sunset"] = strarray[1];
					root["SunAtSouth"] = strarray[2];
					root["CivTwilightStart"] = strarray[3];
					root["CivTwilightEnd"] = strarray[4];
					root["NautTwilightStart"] = strarray[5];
					root["NautTwilightEnd"] = strarray[6];
					root["AstrTwilightStart"] = strarray[7];
					root["AstrTwilightEnd"] = strarray[8];
					root["DayLength"] = strarray[9];
				}
			}

			char szOrderBy[50];
			std::string szQuery;
			bool isAlpha = true;
			const std::string orderBy = order.c_str();
			for (size_t i = 0; i < orderBy.size(); i++) {
				if (!isalpha(orderBy[i])) {
					isAlpha = false;
				}
			}
			if (order.empty() || (!isAlpha)) {
				strcpy(szOrderBy, "A.[Order],A.LastUpdate DESC");
			} else {
				sprintf(szOrderBy, "A.[Order],A.%%s ASC");
			}

			unsigned char tempsign = m_sql.m_tempsign[0];

			bool bHaveUser = false;
			int iUser = -1;
			unsigned int totUserDevices = 0;
			bool bShowScenes = true;
			bHaveUser = (username != "");
			if (bHaveUser)
			{
				iUser = FindUser(username.c_str());
				if (iUser != -1)
				{
					_eUserRights urights = m_users[iUser].userrights;
					if (urights != URIGHTS_ADMIN)
					{
						result = m_sql.safe_query("SELECT DeviceRowID FROM SharedDevices WHERE (SharedUserID == %lu)", m_users[iUser].ID);
						totUserDevices = (unsigned int)result.size();
						bShowScenes = (m_users[iUser].ActiveTabs&(1 << 1)) != 0;
					}
				}
			}

			std::set<std::string> _HiddenDevices;
			bool bAllowDeviceToBeHidden = false;

			int ii = 0;
			if (rfilter == "all")
			{
				if (
					(bShowScenes) &&
					((rused == "all") || (rused == "true"))
					)
				{
					if (rowid != "")
						result = m_sql.safe_query(
							"SELECT A.ID, A.Name, A.nValue, A.LastUpdate, A.Favorite, A.SceneType,"
							" A.Protected, B.XOffset, B.YOffset, B.PlanID, A.Description"
							" FROM Scenes as A"
							" LEFT OUTER JOIN DeviceToPlansMap as B ON (B.DeviceRowID==a.ID) AND (B.DevSceneType==1)"
							" WHERE (A.ID=='%q')",
							rowid.c_str());
					else if ((planID != "") && (planID != "0"))
						result = m_sql.safe_query(
							"SELECT A.ID, A.Name, A.nValue, A.LastUpdate, A.Favorite, A.SceneType,"
							" A.Protected, B.XOffset, B.YOffset, B.PlanID, A.Description"
							" FROM Scenes as A, DeviceToPlansMap as B WHERE (B.PlanID=='%q')"
							" AND (B.DeviceRowID==a.ID) AND (B.DevSceneType==1) ORDER BY B.[Order]",
							planID.c_str());
					else if ((floorID != "") && (floorID != "0"))
						result = m_sql.safe_query(
							"SELECT A.ID, A.Name, A.nValue, A.LastUpdate, A.Favorite, A.SceneType,"
							" A.Protected, B.XOffset, B.YOffset, B.PlanID, A.Description"
							" FROM Scenes as A, DeviceToPlansMap as B, Plans as C"
							" WHERE (C.FloorplanID=='%q') AND (C.ID==B.PlanID) AND (B.DeviceRowID==a.ID)"
							" AND (B.DevSceneType==1) ORDER BY B.[Order]",
							floorID.c_str());
					else {
						szQuery = (
							"SELECT A.ID, A.Name, A.nValue, A.LastUpdate, A.Favorite, A.SceneType,"
							" A.Protected, B.XOffset, B.YOffset, B.PlanID, A.Description"
							" FROM Scenes as A"
							" LEFT OUTER JOIN DeviceToPlansMap as B ON (B.DeviceRowID==a.ID) AND (B.DevSceneType==1)"
							" ORDER BY ");
						szQuery += szOrderBy;
                                                result = m_sql.safe_query(szQuery.c_str(), order.c_str());
					}

					if (!result.empty())
					{
						for (const auto & itt : result)
						{
							std::vector<std::string> sd = itt;

							unsigned char favorite = atoi(sd[4].c_str());
							if ((bFetchFavorites) && (!favorite))
								continue;

							std::string sLastUpdate = sd[3];

							if (iLastUpdate != 0)
							{
								time_t cLastUpdate;
								ParseSQLdatetime(cLastUpdate, tLastUpdate, sLastUpdate, tm1.tm_isdst);
								if (cLastUpdate <= iLastUpdate)
									continue;
							}

							int nValue = atoi(sd[2].c_str());

							unsigned char scenetype = atoi(sd[5].c_str());
							int iProtected = atoi(sd[6].c_str());

							std::string sSceneName = sd[1];
							if (!bDisplayHidden && sSceneName[0] == '$')
							{
								continue;
							}

							if (scenetype == 0)
							{
								root["result"][ii]["Type"] = "Scene";
								root["result"][ii]["TypeImg"] = "scene";
							}
							else
							{
								root["result"][ii]["Type"] = "Group";
								root["result"][ii]["TypeImg"] = "group";
							}

							std::string thisIdx = sd[0];

							if ((ii > 0) && thisIdx == root["result"][ii - 1]["idx"].asString()) {
								std::string typeOfThisOne = root["result"][ii]["Type"].asString();
								if (typeOfThisOne == root["result"][ii - 1]["Type"].asString()) {
									root["result"][ii - 1]["PlanIDs"].append(atoi(sd[9].c_str()));
									continue;
								}
							}

							root["result"][ii]["idx"] = sd[0];
							root["result"][ii]["Name"] = sSceneName;
							root["result"][ii]["Description"] = sd[10];
							root["result"][ii]["Favorite"] = favorite;
							root["result"][ii]["Protected"] = (iProtected != 0);
							root["result"][ii]["LastUpdate"] = sLastUpdate;
							root["result"][ii]["PlanID"] = sd[9].c_str();
							Json::Value jsonArray;
							jsonArray.append(atoi(sd[9].c_str()));
							root["result"][ii]["PlanIDs"] = jsonArray;

							if (nValue == 0)
								root["result"][ii]["Status"] = "Off";
							else if (nValue == 1)
								root["result"][ii]["Status"] = "On";
							else
								root["result"][ii]["Status"] = "Mixed";
							root["result"][ii]["Data"] = root["result"][ii]["Status"];
							uint64_t camIDX = m_mainworker.m_cameras.IsDevSceneInCamera(1, sd[0]);
							root["result"][ii]["UsedByCamera"] = (camIDX != 0) ? true : false;
							if (camIDX != 0) {
								std::stringstream scidx;
								scidx << camIDX;
								root["result"][ii]["CameraIdx"] = scidx.str();
							}
							root["result"][ii]["XOffset"] = atoi(sd[7].c_str());
							root["result"][ii]["YOffset"] = atoi(sd[8].c_str());
							ii++;
						}
					}
				}
			}

			char szData[250];
			if (totUserDevices == 0)
			{
				if (rowid != "")
				{
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used, A.Type, A.SubType,"
						" A.SignalLevel, A.BatteryLevel, A.nValue, A.sValue,"
						" A.LastUpdate, A.Favorite, A.SwitchType, A.HardwareID,"
						" A.AddjValue, A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1, A.StrParam2,"
						" A.Protected, IFNULL(B.XOffset,0), IFNULL(B.YOffset,0), IFNULL(B.PlanID,0), A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus A LEFT OUTER JOIN DeviceToPlansMap as B ON (B.DeviceRowID==a.ID) "
						"WHERE (A.ID=='%q')",
						rowid.c_str());
				}
				else if ((planID != "") && (planID != "0"))
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, B.XOffset, B.YOffset,"
						" B.PlanID, A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, DeviceToPlansMap as B "
						"WHERE (B.PlanID=='%q') AND (B.DeviceRowID==a.ID)"
						" AND (B.DevSceneType==0) ORDER BY B.[Order]",
						planID.c_str());
				else if ((floorID != "") && (floorID != "0"))
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, B.XOffset, B.YOffset,"
						" B.PlanID, A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, DeviceToPlansMap as B,"
						" Plans as C "
						"WHERE (C.FloorplanID=='%q') AND (C.ID==B.PlanID)"
						" AND (B.DeviceRowID==a.ID) AND (B.DevSceneType==0) "
						"ORDER BY B.[Order]",
						floorID.c_str());
				else {
					if (!bDisplayHidden)
					{
						result = m_sql.safe_query("SELECT ID FROM Plans WHERE (Name=='$Hidden Devices')");
						if (!result.empty())
						{
							std::string pID = result[0][0];
							result = m_sql.safe_query("SELECT DeviceRowID FROM DeviceToPlansMap WHERE (PlanID=='%q') AND (DevSceneType==0)",
								pID.c_str());
							if (!result.empty())
							{
								std::vector<std::vector<std::string> >::const_iterator ittP;
								for (ittP = result.begin(); ittP != result.end(); ++ittP)
								{
									_HiddenDevices.insert(ittP[0][0]);
								}
							}
						}
						bAllowDeviceToBeHidden = true;
					}

					if (order.empty() || (!isAlpha))
						strcpy(szOrderBy, "A.[Order],A.LastUpdate DESC");
					else
					{
						sprintf(szOrderBy, "A.[Order],A.%%s ASC");
					}
					if (hardwareid != "") {
						szQuery = (
							"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,A.Type, A.SubType,"
							" A.SignalLevel, A.BatteryLevel, A.nValue, A.sValue,"
							" A.LastUpdate, A.Favorite, A.SwitchType, A.HardwareID,"
							" A.AddjValue, A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
							" A.LastLevel, A.CustomImage, A.StrParam1, A.StrParam2,"
							" A.Protected, IFNULL(B.XOffset,0), IFNULL(B.YOffset,0), IFNULL(B.PlanID,0), A.Description,"
							" A.Options, A.Color "
							"FROM DeviceStatus as A LEFT OUTER JOIN DeviceToPlansMap as B "
							"ON (B.DeviceRowID==a.ID) AND (B.DevSceneType==0) "
							"WHERE (A.HardwareID == %q) "
							"ORDER BY ");
						szQuery += szOrderBy;
						result = m_sql.safe_query(szQuery.c_str(), hardwareid.c_str(), order.c_str());
					}
					else {
						szQuery = (
							"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,A.Type, A.SubType,"
							" A.SignalLevel, A.BatteryLevel, A.nValue, A.sValue,"
							" A.LastUpdate, A.Favorite, A.SwitchType, A.HardwareID,"
							" A.AddjValue, A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
							" A.LastLevel, A.CustomImage, A.StrParam1, A.StrParam2,"
							" A.Protected, IFNULL(B.XOffset,0), IFNULL(B.YOffset,0), IFNULL(B.PlanID,0), A.Description,"
							" A.Options, A.Color "
							"FROM DeviceStatus as A LEFT OUTER JOIN DeviceToPlansMap as B "
							"ON (B.DeviceRowID==a.ID) AND (B.DevSceneType==0) "
							"ORDER BY ");
						szQuery += szOrderBy;
						result = m_sql.safe_query(szQuery.c_str(), order.c_str());
					}
				}
			}
			else
			{
				if (iUser == -1) {
					return;
				}
				if (rowid != "")
				{
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, 0 as XOffset,"
						" 0 as YOffset, 0 as PlanID, A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, SharedDevices as B "
						"WHERE (B.DeviceRowID==a.ID)"
						" AND (B.SharedUserID==%lu) AND (A.ID=='%q')",
						m_users[iUser].ID, rowid.c_str());
				}
				else if ((planID != "") && (planID != "0"))
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, C.XOffset,"
						" C.YOffset, C.PlanID, A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, SharedDevices as B,"
						" DeviceToPlansMap as C "
						"WHERE (C.PlanID=='%q') AND (C.DeviceRowID==a.ID)"
						" AND (B.DeviceRowID==a.ID) "
						"AND (B.SharedUserID==%lu) ORDER BY C.[Order]",
						planID.c_str(), m_users[iUser].ID);
				else if ((floorID != "") && (floorID != "0"))
					result = m_sql.safe_query(
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, C.XOffset, C.YOffset,"
						" C.PlanID, A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, SharedDevices as B,"
						" DeviceToPlansMap as C, Plans as D "
						"WHERE (D.FloorplanID=='%q') AND (D.ID==C.PlanID)"
						" AND (C.DeviceRowID==a.ID) AND (B.DeviceRowID==a.ID)"
						" AND (B.SharedUserID==%lu) ORDER BY C.[Order]",
						floorID.c_str(), m_users[iUser].ID);
				else {
					if (!bDisplayHidden)
					{
						result = m_sql.safe_query("SELECT ID FROM Plans WHERE (Name=='$Hidden Devices')");
						if (!result.empty())
						{
							std::string pID = result[0][0];
							result = m_sql.safe_query("SELECT DeviceRowID FROM DeviceToPlansMap WHERE (PlanID=='%q')  AND (DevSceneType==0)",
								pID.c_str());
							if (!result.empty())
							{
								std::vector<std::vector<std::string> >::const_iterator ittP;
								for (ittP = result.begin(); ittP != result.end(); ++ittP)
								{
									_HiddenDevices.insert(ittP[0][0]);
								}
							}
						}
						bAllowDeviceToBeHidden = true;
					}

					if (order.empty() || (!isAlpha))
					{
						strcpy(szOrderBy, "A.[Order],A.LastUpdate DESC");
					}
					else
					{
						sprintf(szOrderBy, "A.[Order],A.%%s ASC");
					}
					szQuery = (
						"SELECT A.ID, A.DeviceID, A.Unit, A.Name, A.Used,"
						" A.Type, A.SubType, A.SignalLevel, A.BatteryLevel,"
						" A.nValue, A.sValue, A.LastUpdate, A.Favorite,"
						" A.SwitchType, A.HardwareID, A.AddjValue,"
						" A.AddjMulti, A.AddjValue2, A.AddjMulti2,"
						" A.LastLevel, A.CustomImage, A.StrParam1,"
						" A.StrParam2, A.Protected, IFNULL(C.XOffset,0),"
						" IFNULL(C.YOffset,0), IFNULL(C.PlanID,0), A.Description,"
						" A.Options, A.Color "
						"FROM DeviceStatus as A, SharedDevices as B "
						"LEFT OUTER JOIN DeviceToPlansMap as C  ON (C.DeviceRowID==A.ID)"
						"WHERE (B.DeviceRowID==A.ID)"
						" AND (B.SharedUserID==%lu) ORDER BY ");
					szQuery += szOrderBy;
					result = m_sql.safe_query(szQuery.c_str(), m_users[iUser].ID, order.c_str());
				}
			}

			if (!result.empty())
			{
				for (const auto & itt : result)
				{
					std::vector<std::string> sd = itt;

					unsigned char favorite = atoi(sd[12].c_str());
					if ((planID != "") && (planID != "0"))
						favorite = 1;

					if ((bFetchFavorites) && (!favorite))
						continue;

					std::string sDeviceName = sd[3];

					if (!bDisplayHidden)
					{
						if (_HiddenDevices.find(sd[0]) != _HiddenDevices.end())
							continue;
						if (sDeviceName[0] == '$')
						{
							if (bAllowDeviceToBeHidden)
								continue;
							if (planID.size() > 0)
								sDeviceName = sDeviceName.substr(1);
						}
					}
					int hardwareID = atoi(sd[14].c_str());
					std::map<int, _tHardwareListInt>::iterator hItt = _hardwareNames.find(hardwareID);
					if (hItt != _hardwareNames.end())
					{
						if ((!bDisplayDisabled) && (!(*hItt).second.Enabled))
							continue;
					}

					unsigned int dType = atoi(sd[5].c_str());
					unsigned int dSubType = atoi(sd[6].c_str());
					unsigned int used = atoi(sd[4].c_str());
					int nValue = atoi(sd[9].c_str());
					std::string sValue = sd[10];
					std::string sLastUpdate = sd[11];
					if (sLastUpdate.size() > 19)
						sLastUpdate = sLastUpdate.substr(0, 19);

					if (iLastUpdate != 0)
					{
						time_t cLastUpdate;
						ParseSQLdatetime(cLastUpdate, tLastUpdate, sLastUpdate, tm1.tm_isdst);
						if (cLastUpdate <= iLastUpdate)
							continue;
					}

					_eSwitchType switchtype = (_eSwitchType)atoi(sd[13].c_str());
					_eMeterType metertype = (_eMeterType)switchtype;
					double AddjValue = atof(sd[15].c_str());
					double AddjMulti = atof(sd[16].c_str());
					double AddjValue2 = atof(sd[17].c_str());
					double AddjMulti2 = atof(sd[18].c_str());
					int LastLevel = atoi(sd[19].c_str());
					int CustomImage = atoi(sd[20].c_str());
					std::string strParam1 = base64_encode(sd[21]);
					std::string strParam2 = base64_encode(sd[22]);
					int iProtected = atoi(sd[23].c_str());

					std::string Description = sd[27];
					std::string sOptions = sd[28];
					std::string sColor = sd[29];
					std::map<std::string, std::string> options = m_sql.BuildDeviceOptions(sOptions);

					struct tm ntime;
					time_t checktime;
					ParseSQLdatetime(checktime, ntime, sLastUpdate, tm1.tm_isdst);
					bool bHaveTimeout = (now - checktime >= SensorTimeOut * 60);

					if (dType == pTypeTEMP_RAIN)
						continue; //dont want you for now

					if ((rused == "true") && (!used))
						continue;

					if (
						(rused == "false") &&
						(used)
						)
						continue;
					if (rfilter != "")
					{
						if (rfilter == "light")
						{
							if (
								(dType != pTypeLighting1) &&
								(dType != pTypeLighting2) &&
								(dType != pTypeLighting3) &&
								(dType != pTypeLighting4) &&
								(dType != pTypeLighting5) &&
								(dType != pTypeLighting6) &&
								(dType != pTypeFan) &&
								(dType != pTypeColorSwitch) &&
								(dType != pTypeSecurity1) &&
								(dType != pTypeSecurity2) &&
								(dType != pTypeEvohome) &&
								(dType != pTypeEvohomeRelay) &&
								(dType != pTypeCurtain) &&
								(dType != pTypeBlinds) &&
								(dType != pTypeRFY) &&
								(dType != pTypeChime) &&
								(dType != pTypeThermostat2) &&
								(dType != pTypeThermostat3) &&
								(dType != pTypeThermostat4) &&
								(dType != pTypeRemote) &&
								(dType != pTypeGeneralSwitch) &&
								(dType != pTypeHomeConfort) &&
								(dType != pTypeChime) &&
								(dType != pTypeFS20) &&
								(!((dType == pTypeRego6XXValue) && (dSubType == sTypeRego6XXStatus))) &&
								(!((dType == pTypeRadiator1) && (dSubType == sTypeSmartwaresSwitchRadiator)))
								)
								continue;
						}
						else if (rfilter == "temp")
						{
							if (
								(dType != pTypeTEMP) &&
								(dType != pTypeHUM) &&
								(dType != pTypeTEMP_HUM) &&
								(dType != pTypeTEMP_HUM_BARO) &&
								(dType != pTypeTEMP_BARO) &&
								(dType != pTypeEvohomeZone) &&
								(dType != pTypeEvohomeWater) &&
								(!((dType == pTypeWIND) && (dSubType == sTypeWIND4))) &&
								(!((dType == pTypeUV) && (dSubType == sTypeUV3))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeSystemTemp))) &&
								(dType != pTypeThermostat1) &&
								(!((dType == pTypeRFXSensor) && (dSubType == sTypeRFXSensorTemp))) &&
								(dType != pTypeRego6XXTemp)
								)
								continue;
						}
						else if (rfilter == "weather")
						{
							if (
								(dType != pTypeWIND) &&
								(dType != pTypeRAIN) &&
								(dType != pTypeTEMP_HUM_BARO) &&
								(dType != pTypeTEMP_BARO) &&
								(dType != pTypeUV) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeVisibility))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeBaro))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeSolarRadiation)))
								)
								continue;
						}
						else if (rfilter == "utility")
						{
							if (
								(dType != pTypeRFXMeter) &&
								(!((dType == pTypeRFXSensor) && (dSubType == sTypeRFXSensorAD))) &&
								(!((dType == pTypeRFXSensor) && (dSubType == sTypeRFXSensorVolt))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeVoltage))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeCurrent))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeTextStatus))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeAlert))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypePressure))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeSoilMoisture))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeLeafWetness))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypePercentage))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeWaterflow))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeCustom))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeFan))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeSoundLevel))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeZWaveClock))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeZWaveThermostatMode))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeZWaveThermostatFanMode))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeDistance))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeCounterIncremental))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeManagedCounter))) &&
								(!((dType == pTypeGeneral) && (dSubType == sTypeKwh))) &&
								(dType != pTypeCURRENT) &&
								(dType != pTypeCURRENTENERGY) &&
								(dType != pTypeENERGY) &&
								(dType != pTypePOWER) &&
								(dType != pTypeP1Power) &&
								(dType != pTypeP1Gas) &&
								(dType != pTypeYouLess) &&
								(dType != pTypeAirQuality) &&
								(dType != pTypeLux) &&
								(dType != pTypeUsage) &&
								(!((dType == pTypeRego6XXValue) && (dSubType == sTypeRego6XXCounter))) &&
								(!((dType == pTypeThermostat) && (dSubType == sTypeThermSetpoint))) &&
								(dType != pTypeWEIGHT) &&
								(!((dType == pTypeRadiator1) && (dSubType == sTypeSmartwares)))
								)
								continue;
						}
						else if (rfilter == "wind")
						{
							if (
								(dType != pTypeWIND)
								)
								continue;
						}
						else if (rfilter == "rain")
						{
							if (
								(dType != pTypeRAIN)
								)
								continue;
						}
						else if (rfilter == "uv")
						{
							if (
								(dType != pTypeUV)
								)
								continue;
						}
						else if (rfilter == "baro")
						{
							if (
								(dType != pTypeTEMP_HUM_BARO) &&
								(dType != pTypeTEMP_BARO)
								)
								continue;
						}
						else if (rfilter == "zwavealarms")
						{
							if (!((dType == pTypeGeneral) && (dSubType == sTypeZWaveAlarm)))
								continue;
						}
					}

					std::string thisIdx = sd[0];
					int devIdx = atoi(thisIdx.c_str());

					if ((ii > 0) && thisIdx == root["result"][ii - 1]["idx"].asString()) {
						std::string typeOfThisOne = RFX_Type_Desc(dType, 1);
						if (typeOfThisOne == root["result"][ii - 1]["Type"].asString()) {
							root["result"][ii - 1]["PlanIDs"].append(atoi(sd[26].c_str()));
							continue;
						}
					}

					root["result"][ii]["HardwareID"] = hardwareID;
					if (_hardwareNames.find(hardwareID) == _hardwareNames.end())
					{
						root["result"][ii]["HardwareName"] = "Unknown?";
						root["result"][ii]["HardwareTypeVal"] = 0;
						root["result"][ii]["HardwareType"] = "Unknown?";
					}
					else
					{
						root["result"][ii]["HardwareName"] = _hardwareNames[hardwareID].Name;
						root["result"][ii]["HardwareTypeVal"] = _hardwareNames[hardwareID].HardwareTypeVal;
						root["result"][ii]["HardwareType"] = _hardwareNames[hardwareID].HardwareType;
					}
					root["result"][ii]["idx"] = sd[0];
					root["result"][ii]["Protected"] = (iProtected != 0);

					CDomoticzHardwareBase *pHardware = m_mainworker.GetHardware(hardwareID);
					if (pHardware != NULL)
					{
						if (pHardware->HwdType == HTYPE_SolarEdgeAPI)
						{
							int seSensorTimeOut = 60 * 24 * 60;
							bHaveTimeout = (now - checktime >= seSensorTimeOut * 60);
						}
						else if (pHardware->HwdType == HTYPE_Wunderground)
						{
							CWunderground *pWHardware = reinterpret_cast<CWunderground *>(pHardware);
							std::string forecast_url = pWHardware->GetForecastURL();
							if (forecast_url != "")
							{
								root["result"][ii]["forecast_url"] = base64_encode(forecast_url);
							}
						}
						else if (pHardware->HwdType == HTYPE_DarkSky)
						{
							CDarkSky *pWHardware = reinterpret_cast<CDarkSky*>(pHardware);
							std::string forecast_url = pWHardware->GetForecastURL();
							if (forecast_url != "")
							{
								root["result"][ii]["forecast_url"] = base64_encode(forecast_url);
							}
						}
						else if (pHardware->HwdType == HTYPE_AccuWeather)
						{
							CAccuWeather *pWHardware = reinterpret_cast<CAccuWeather*>(pHardware);
							std::string forecast_url = pWHardware->GetForecastURL();
							if (forecast_url != "")
							{
								root["result"][ii]["forecast_url"] = base64_encode(forecast_url);
							}
						}
						else if (pHardware->HwdType == HTYPE_OpenWeatherMap)
						{
							COpenWeatherMap *pWHardware = reinterpret_cast<COpenWeatherMap*>(pHardware);
							std::string forecast_url = pWHardware->GetForecastURL();
							if (forecast_url != "")
							{
								root["result"][ii]["forecast_url"] = base64_encode(forecast_url);
							}
						}
					}

					if ((pHardware != NULL) && (pHardware->HwdType == HTYPE_PythonPlugin))
					{
						root["result"][ii]["ID"] = sd[1];
					}
					else
					{
						sprintf(szData, "%04X", (unsigned int)atoi(sd[1].c_str()));
						if (
							(dType == pTypeTEMP) ||
							(dType == pTypeTEMP_BARO) ||
							(dType == pTypeTEMP_HUM) ||
							(dType == pTypeTEMP_HUM_BARO) ||
							(dType == pTypeBARO) ||
							(dType == pTypeHUM) ||
							(dType == pTypeWIND) ||
							(dType == pTypeRAIN) ||
							(dType == pTypeUV) ||
							(dType == pTypeCURRENT) ||
							(dType == pTypeCURRENTENERGY) ||
							(dType == pTypeENERGY) ||
							(dType == pTypeRFXMeter) ||
							(dType == pTypeAirQuality) ||
							(dType == pTypeRFXSensor) ||
							(dType == pTypeP1Power) ||
							(dType == pTypeP1Gas)
							)
						{
							root["result"][ii]["ID"] = szData;
						}
						else
						{
							root["result"][ii]["ID"] = sd[1];
						}
					}
					root["result"][ii]["Unit"] = atoi(sd[2].c_str());
					root["result"][ii]["Type"] = RFX_Type_Desc(dType, 1);
					root["result"][ii]["SubType"] = RFX_Type_SubType_Desc(dType, dSubType);
					root["result"][ii]["TypeImg"] = RFX_Type_Desc(dType, 2);
					root["result"][ii]["Name"] = sDeviceName;
					root["result"][ii]["Description"] = Description;
					root["result"][ii]["Used"] = used;
					root["result"][ii]["Favorite"] = favorite;

					int iSignalLevel = atoi(sd[7].c_str());
					if (iSignalLevel < 12)
						root["result"][ii]["SignalLevel"] = iSignalLevel;
					else
						root["result"][ii]["SignalLevel"] = "-";
					root["result"][ii]["BatteryLevel"] = atoi(sd[8].c_str());
					root["result"][ii]["LastUpdate"] = sLastUpdate;
					root["result"][ii]["CustomImage"] = CustomImage;
					root["result"][ii]["XOffset"] = sd[24].c_str();
					root["result"][ii]["YOffset"] = sd[25].c_str();
					root["result"][ii]["PlanID"] = sd[26].c_str();
					Json::Value jsonArray;
					jsonArray.append(atoi(sd[26].c_str()));
					root["result"][ii]["PlanIDs"] = jsonArray;
					root["result"][ii]["AddjValue"] = AddjValue;
					root["result"][ii]["AddjMulti"] = AddjMulti;
					root["result"][ii]["AddjValue2"] = AddjValue2;
					root["result"][ii]["AddjMulti2"] = AddjMulti2;

					std::stringstream s_data;
					s_data << int(nValue) << ", " << sValue;
					root["result"][ii]["Data"] = s_data.str();

					root["result"][ii]["Notifications"] = (m_notifications.HasNotifications(sd[0]) == true) ? "true" : "false";
					root["result"][ii]["ShowNotifications"] = true;

					bool bHasTimers = false;

					if (
						(dType == pTypeLighting1) ||
						(dType == pTypeLighting2) ||
						(dType == pTypeLighting3) ||
						(dType == pTypeLighting4) ||
						(dType == pTypeLighting5) ||
						(dType == pTypeLighting6) ||
						(dType == pTypeFan) ||
						(dType == pTypeColorSwitch) ||
						(dType == pTypeCurtain) ||
						(dType == pTypeBlinds) ||
						(dType == pTypeRFY) ||
						(dType == pTypeChime) ||
						(dType == pTypeThermostat2) ||
						(dType == pTypeThermostat3) ||
						(dType == pTypeThermostat4) ||
						(dType == pTypeRemote) ||
						(dType == pTypeGeneralSwitch) ||
						(dType == pTypeHomeConfort) ||
						(dType == pTypeFS20) ||
						((dType == pTypeRadiator1) && (dSubType == sTypeSmartwaresSwitchRadiator)) ||
						((dType == pTypeRego6XXValue) && (dSubType == sTypeRego6XXStatus))
						)
					{
						bHasTimers = m_sql.HasTimers(sd[0]);

						bHaveTimeout = false;
#ifdef WITH_OPENZWAVE
						if (pHardware != NULL)
						{
							if (pHardware->HwdType == HTYPE_OpenZWave)
							{
								COpenZWave *pZWave = reinterpret_cast<COpenZWave*>(pHardware);
								unsigned long ID;
								std::stringstream s_strid;
								s_strid << std::hex << sd[1];
								s_strid >> ID;
								int nodeID = (ID & 0x0000FF00) >> 8;
								bHaveTimeout = pZWave->HasNodeFailed(nodeID);
							}
						}
#endif
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;

						std::string lstatus = "";
						int llevel = 0;
						bool bHaveDimmer = false;
						bool bHaveGroupCmd = false;
						int maxDimLevel = 0;

						GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

						root["result"][ii]["Status"] = lstatus;
						root["result"][ii]["StrParam1"] = strParam1;
						root["result"][ii]["StrParam2"] = strParam2;

						std::string IconFile = "Light";
						std::map<int, int>::const_iterator ittIcon = m_custom_light_icons_lookup.find(CustomImage);
						if (ittIcon != m_custom_light_icons_lookup.end())
						{
							IconFile = m_custom_light_icons[ittIcon->second].RootFile;
						}
						root["result"][ii]["Image"] = IconFile;

						if (switchtype == STYPE_Dimmer)
						{
							root["result"][ii]["Level"] = LastLevel;
							int iLevel = round((float(maxDimLevel) / 100.0f)*LastLevel);
							root["result"][ii]["LevelInt"] = iLevel;
							if ((dType == pTypeColorSwitch) ||
							    (dType == pTypeLighting5 && dSubType == sTypeTRC02) ||
							    (dType == pTypeLighting5 && dSubType == sTypeTRC02_2) ||
							    (dType == pTypeGeneralSwitch && dSubType == sSwitchTypeTRC02) ||
							    (dType == pTypeGeneralSwitch && dSubType == sSwitchTypeTRC02_2))
							{
								_tColor color(sColor);
								std::string jsonColor = color.toJSONString();
								root["result"][ii]["Color"] = jsonColor;
								llevel = LastLevel;
								if (lstatus == "Set Level" || lstatus == "Set Color")
								{
									sprintf(szTmp, "Set Level: %d %%", LastLevel);
									root["result"][ii]["Status"] = szTmp;
								}
							}
						}
						else
						{
							root["result"][ii]["Level"] = llevel;
							root["result"][ii]["LevelInt"] = atoi(sValue.c_str());
						}
						root["result"][ii]["HaveDimmer"] = bHaveDimmer;
						std::string DimmerType = "none";
						if (switchtype == STYPE_Dimmer)
						{
							DimmerType = "abs";
							if (_hardwareNames.find(hardwareID) != _hardwareNames.end())
							{
								if (_hardwareNames[hardwareID].HardwareTypeVal == HTYPE_LimitlessLights &&
								    atoi(_hardwareNames[hardwareID].Mode2.c_str()) != CLimitLess::LBTYPE_V6 &&
									(atoi(_hardwareNames[hardwareID].Mode1.c_str()) == sTypeColor_RGB ||
									 atoi(_hardwareNames[hardwareID].Mode1.c_str()) == sTypeColor_White ||
									 atoi(_hardwareNames[hardwareID].Mode1.c_str()) == sTypeColor_CW_WW))
								{
									DimmerType = "rel";
								}
							}
						}
						root["result"][ii]["DimmerType"] = DimmerType;
						root["result"][ii]["MaxDimLevel"] = maxDimLevel;
						root["result"][ii]["HaveGroupCmd"] = bHaveGroupCmd;
						root["result"][ii]["SwitchType"] = Switch_Type_Desc(switchtype);
						root["result"][ii]["SwitchTypeVal"] = switchtype;
						uint64_t camIDX = m_mainworker.m_cameras.IsDevSceneInCamera(0, sd[0]);
						root["result"][ii]["UsedByCamera"] = (camIDX != 0) ? true : false;
						if (camIDX != 0) {
							std::stringstream scidx;
							scidx << camIDX;
							root["result"][ii]["CameraIdx"] = scidx.str();
						}

						bool bIsSubDevice = false;
						std::vector<std::vector<std::string> > resultSD;
						resultSD = m_sql.safe_query("SELECT ID FROM LightSubDevices WHERE (DeviceRowID=='%q')",
							sd[0].c_str());
						bIsSubDevice = (resultSD.size() > 0);

						root["result"][ii]["IsSubDevice"] = bIsSubDevice;

						if (switchtype == STYPE_Doorbell)
						{
							root["result"][ii]["TypeImg"] = "doorbell";
							root["result"][ii]["Status"] = "";//"Pressed";
						}
						else if (switchtype == STYPE_DoorContact)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Door";
							}
							root["result"][ii]["TypeImg"] = "door";
							bool bIsOn = IsLightSwitchOn(lstatus);
							root["result"][ii]["InternalState"] = (bIsOn == true) ? "Open" : "Closed";
							if (bIsOn) {
								lstatus = "Open";
							}
							else {
								lstatus = "Closed";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_DoorLock)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Door";
							}
							root["result"][ii]["TypeImg"] = "door";
							bool bIsOn = IsLightSwitchOn(lstatus);
							root["result"][ii]["InternalState"] = (bIsOn == true) ? "Locked" : "Unlocked";
							if (bIsOn) {
								lstatus = "Locked";
							}
							else {
								lstatus = "Unlocked";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_DoorLockInverted)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Door";
							}
							root["result"][ii]["TypeImg"] = "door";
							bool bIsOn = IsLightSwitchOn(lstatus);
							root["result"][ii]["InternalState"] = (bIsOn == true) ? "Unlocked" : "Locked";
							if (bIsOn) {
								lstatus = "Unlocked";
							}
							else {
								lstatus = "Locked";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_PushOn)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Push";
							}
							root["result"][ii]["TypeImg"] = "push";
							root["result"][ii]["Status"] = "";
							root["result"][ii]["InternalState"] = (IsLightSwitchOn(lstatus) == true) ? "On" : "Off";
						}
						else if (switchtype == STYPE_PushOff)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Push";
							}
							root["result"][ii]["TypeImg"] = "push";
							root["result"][ii]["Status"] = "";
							root["result"][ii]["TypeImg"] = "pushoff";
						}
						else if (switchtype == STYPE_X10Siren)
							root["result"][ii]["TypeImg"] = "siren";
						else if (switchtype == STYPE_SMOKEDETECTOR)
						{
							root["result"][ii]["TypeImg"] = "smoke";
							root["result"][ii]["SwitchTypeVal"] = STYPE_SMOKEDETECTOR;
							root["result"][ii]["SwitchType"] = Switch_Type_Desc(STYPE_SMOKEDETECTOR);
						}
						else if (switchtype == STYPE_Contact)
						{
							if (CustomImage == 0)
							{
								root["result"][ii]["Image"] = "Contact";
							}
							root["result"][ii]["TypeImg"] = "contact";
							bool bIsOn = IsLightSwitchOn(lstatus);
							if (bIsOn) {
								lstatus = "Open";
							}
							else {
								lstatus = "Closed";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_Media)
						{
							if ((pHardware != NULL) && (pHardware->HwdType == HTYPE_LogitechMediaServer))
								root["result"][ii]["TypeImg"] = "LogitechMediaServer";
							else
								root["result"][ii]["TypeImg"] = "Media";
							root["result"][ii]["Status"] = Media_Player_States((_eMediaStatus)nValue);
							lstatus = sValue;
						}
						else if (
							(switchtype == STYPE_Blinds) ||
							(switchtype == STYPE_VenetianBlindsUS) ||
							(switchtype == STYPE_VenetianBlindsEU)
							)
						{
							root["result"][ii]["TypeImg"] = "blinds";
							if ((lstatus == "On") || (lstatus == "Close inline relay")) {
								lstatus = "Closed";
							}
							else if ((lstatus == "Stop") || (lstatus == "Stop inline relay")) {
								lstatus = "Stopped";
							}
							else {
								lstatus = "Open";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_BlindsInverted)
						{
							root["result"][ii]["TypeImg"] = "blinds";
							if (lstatus == "On") {
								lstatus = "Open";
							}
							else {
								lstatus = "Closed";
							}
							root["result"][ii]["Status"] = lstatus;
						}
						else if ((switchtype == STYPE_BlindsPercentage) || (switchtype == STYPE_BlindsPercentageInverted))
						{
							root["result"][ii]["TypeImg"] = "blinds";
							root["result"][ii]["Level"] = LastLevel;
							int iLevel = round((float(maxDimLevel) / 100.0f)*LastLevel);
							root["result"][ii]["LevelInt"] = iLevel;
							if (lstatus == "On") {
								lstatus = (switchtype == STYPE_BlindsPercentage) ? "Closed" : "Open";
							}
							else if (lstatus == "Off") {
								lstatus = (switchtype == STYPE_BlindsPercentage) ? "Open" : "Closed";
							}

							root["result"][ii]["Status"] = lstatus;
						}
						else if (switchtype == STYPE_Dimmer)
						{
							root["result"][ii]["TypeImg"] = "dimmer";
						}
						else if (switchtype == STYPE_Motion)
						{
							root["result"][ii]["TypeImg"] = "motion";
						}
						else if (switchtype == STYPE_Selector)
						{
							std::string selectorStyle = options["SelectorStyle"];
							std::string levelOffHidden = options["LevelOffHidden"];
							std::string levelNames = options["LevelNames"];
							std::string levelActions = options["LevelActions"];
							if (selectorStyle.empty()) {
								selectorStyle.assign("0"); // default is 'button set'
							}
							if (levelOffHidden.empty()) {
								levelOffHidden.assign("false"); // default is 'not hidden'
							}
							if (levelNames.empty()) {
								levelNames.assign("Off"); // default is Off only
							}
							root["result"][ii]["TypeImg"] = "Light";
							root["result"][ii]["SelectorStyle"] = atoi(selectorStyle.c_str());
							root["result"][ii]["LevelOffHidden"] = (levelOffHidden == "true");
							root["result"][ii]["LevelNames"] = base64_encode(levelNames);
							root["result"][ii]["LevelActions"] = base64_encode(levelActions);
						}
						sprintf(szData, "%s", lstatus.c_str());
						root["result"][ii]["Data"] = szData;
					}
					else if (dType == pTypeSecurity1)
					{
						std::string lstatus = "";
						int llevel = 0;
						bool bHaveDimmer = false;
						bool bHaveGroupCmd = false;
						int maxDimLevel = 0;

						GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

						root["result"][ii]["Status"] = lstatus;
						root["result"][ii]["HaveDimmer"] = bHaveDimmer;
						root["result"][ii]["MaxDimLevel"] = maxDimLevel;
						root["result"][ii]["HaveGroupCmd"] = bHaveGroupCmd;
						root["result"][ii]["SwitchType"] = "Security";
						root["result"][ii]["SwitchTypeVal"] = switchtype; //was 0?;
						root["result"][ii]["TypeImg"] = "security";
						root["result"][ii]["StrParam1"] = strParam1;
						root["result"][ii]["StrParam2"] = strParam2;
						root["result"][ii]["Protected"] = (iProtected != 0);

						if ((dSubType == sTypeKD101) || (dSubType == sTypeSA30) || (switchtype == STYPE_SMOKEDETECTOR))
						{
							root["result"][ii]["SwitchTypeVal"] = STYPE_SMOKEDETECTOR;
							root["result"][ii]["TypeImg"] = "smoke";
							root["result"][ii]["SwitchType"] = Switch_Type_Desc(STYPE_SMOKEDETECTOR);
						}
						sprintf(szData, "%s", lstatus.c_str());
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = false;
					}
					else if (dType == pTypeSecurity2)
					{
						std::string lstatus = "";
						int llevel = 0;
						bool bHaveDimmer = false;
						bool bHaveGroupCmd = false;
						int maxDimLevel = 0;

						GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

						root["result"][ii]["Status"] = lstatus;
						root["result"][ii]["HaveDimmer"] = bHaveDimmer;
						root["result"][ii]["MaxDimLevel"] = maxDimLevel;
						root["result"][ii]["HaveGroupCmd"] = bHaveGroupCmd;
						root["result"][ii]["SwitchType"] = "Security";
						root["result"][ii]["SwitchTypeVal"] = switchtype; //was 0?;
						root["result"][ii]["TypeImg"] = "security";
						root["result"][ii]["StrParam1"] = strParam1;
						root["result"][ii]["StrParam2"] = strParam2;
						root["result"][ii]["Protected"] = (iProtected != 0);
						sprintf(szData, "%s", lstatus.c_str());
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = false;
					}
					else if (dType == pTypeEvohome || dType == pTypeEvohomeRelay)
					{
						std::string lstatus = "";
						int llevel = 0;
						bool bHaveDimmer = false;
						bool bHaveGroupCmd = false;
						int maxDimLevel = 0;

						GetLightStatus(dType, dSubType, switchtype, nValue, sValue, lstatus, llevel, bHaveDimmer, maxDimLevel, bHaveGroupCmd);

						root["result"][ii]["Status"] = lstatus;
						root["result"][ii]["HaveDimmer"] = bHaveDimmer;
						root["result"][ii]["MaxDimLevel"] = maxDimLevel;
						root["result"][ii]["HaveGroupCmd"] = bHaveGroupCmd;
						root["result"][ii]["SwitchType"] = "evohome";
						root["result"][ii]["SwitchTypeVal"] = switchtype; //was 0?;
						root["result"][ii]["TypeImg"] = "override_mini";
						root["result"][ii]["StrParam1"] = strParam1;
						root["result"][ii]["StrParam2"] = strParam2;
						root["result"][ii]["Protected"] = (iProtected != 0);

						sprintf(szData, "%s", lstatus.c_str());
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = false;

						if (dType == pTypeEvohomeRelay)
						{
							root["result"][ii]["SwitchType"] = "TPI";
							root["result"][ii]["Level"] = llevel;
							root["result"][ii]["LevelInt"] = atoi(sValue.c_str());
							if (root["result"][ii]["Unit"].asInt() > 100)
								root["result"][ii]["Protected"] = true;

							sprintf(szData, "%s: %d", lstatus.c_str(), atoi(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
						}
					}
					else if ((dType == pTypeEvohomeZone) || (dType == pTypeEvohomeWater))
					{
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						root["result"][ii]["TypeImg"] = "override_mini";

						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() >= 3)
						{
							int i = 0;
							double tempCelcius = atof(strarray[i++].c_str());
							double temp = ConvertTemperature(tempCelcius, tempsign);
							double tempSetPoint;
							root["result"][ii]["Temp"] = temp;
							if (dType == pTypeEvohomeZone)
							{
								tempCelcius = atof(strarray[i++].c_str());
								tempSetPoint = ConvertTemperature(tempCelcius, tempsign);
								root["result"][ii]["SetPoint"] = tempSetPoint;
							}
							else
								root["result"][ii]["State"] = strarray[i++];

							std::string strstatus = strarray[i++];
							root["result"][ii]["Status"] = strstatus;

							if ((dType == pTypeEvohomeZone || dType == pTypeEvohomeWater) && strarray.size() >= 4)
							{
								root["result"][ii]["Until"] = strarray[i++];
							}
							if (dType == pTypeEvohomeZone)
							{
								if (tempCelcius == 325.1)
									sprintf(szTmp, "Off");
								else
									sprintf(szTmp, "%.1f %c", tempSetPoint, tempsign);
								if (strarray.size() >= 4)
									sprintf(szData, "%.1f %c, (%s), %s until %s", temp, tempsign, szTmp, strstatus.c_str(), strarray[3].c_str());
								else
									sprintf(szData, "%.1f %c, (%s), %s", temp, tempsign, szTmp, strstatus.c_str());
							}
							else
								if (strarray.size() >= 4)
									sprintf(szData, "%.1f %c, %s, %s until %s", temp, tempsign, strarray[1].c_str(), strstatus.c_str(), strarray[3].c_str());
								else
									sprintf(szData, "%.1f %c, %s, %s", temp, tempsign, strarray[1].c_str(), strstatus.c_str());
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if ((dType == pTypeTEMP) || (dType == pTypeRego6XXTemp))
					{
						double tvalue = ConvertTemperature(atof(sValue.c_str()), tempsign);
						root["result"][ii]["Temp"] = tvalue;
						sprintf(szData, "%.1f %c", tvalue, tempsign);
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;

						_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
						uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
						if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
						{
							tstate = m_mainworker.m_trend_calculator[tID].m_state;
						}
						root["result"][ii]["trend"] = (int)tstate;
					}
					else if (dType == pTypeThermostat1)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 4)
						{
							double tvalue = ConvertTemperature(atof(strarray[0].c_str()), tempsign);
							root["result"][ii]["Temp"] = tvalue;
							sprintf(szData, "%.1f %c", tvalue, tempsign);
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if ((dType == pTypeRFXSensor) && (dSubType == sTypeRFXSensorTemp))
					{
						double tvalue = ConvertTemperature(atof(sValue.c_str()), tempsign);
						root["result"][ii]["Temp"] = tvalue;
						sprintf(szData, "%.1f %c", tvalue, tempsign);
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["TypeImg"] = "temperature";
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
						uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
						if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
						{
							tstate = m_mainworker.m_trend_calculator[tID].m_state;
						}
						root["result"][ii]["trend"] = (int)tstate;
					}
					else if (dType == pTypeHUM)
					{
						root["result"][ii]["Humidity"] = nValue;
						root["result"][ii]["HumidityStatus"] = RFX_Humidity_Status_Desc(atoi(sValue.c_str()));
						sprintf(szData, "Humidity %d %%", nValue);
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
					}
					else if (dType == pTypeTEMP_HUM)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 3)
						{
							double tempCelcius = atof(strarray[0].c_str());
							double temp = ConvertTemperature(tempCelcius, tempsign);
							int humidity = atoi(strarray[1].c_str());

							root["result"][ii]["Temp"] = temp;
							root["result"][ii]["Humidity"] = humidity;
							root["result"][ii]["HumidityStatus"] = RFX_Humidity_Status_Desc(atoi(strarray[2].c_str()));
							sprintf(szData, "%.1f %c, %d %%", temp, tempsign, atoi(strarray[1].c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;


							sprintf(szTmp, "%.2f", ConvertTemperature(CalculateDewPoint(tempCelcius, humidity), tempsign));
							root["result"][ii]["DewPoint"] = szTmp;

							_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
							uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
							if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
							{
								tstate = m_mainworker.m_trend_calculator[tID].m_state;
							}
							root["result"][ii]["trend"] = (int)tstate;
						}
					}
					else if (dType == pTypeTEMP_HUM_BARO)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 5)
						{
							double tempCelcius = atof(strarray[0].c_str());
							double temp = ConvertTemperature(tempCelcius, tempsign);
							int humidity = atoi(strarray[1].c_str());

							root["result"][ii]["Temp"] = temp;
							root["result"][ii]["Humidity"] = humidity;
							root["result"][ii]["HumidityStatus"] = RFX_Humidity_Status_Desc(atoi(strarray[2].c_str()));
							root["result"][ii]["Forecast"] = atoi(strarray[4].c_str());

							sprintf(szTmp, "%.2f", ConvertTemperature(CalculateDewPoint(tempCelcius, humidity), tempsign));
							root["result"][ii]["DewPoint"] = szTmp;

							if (dSubType == sTypeTHBFloat)
							{
								root["result"][ii]["Barometer"] = atof(strarray[3].c_str());
								root["result"][ii]["ForecastStr"] = RFX_WSForecast_Desc(atoi(strarray[4].c_str()));
							}
							else
							{
								root["result"][ii]["Barometer"] = atoi(strarray[3].c_str());
								root["result"][ii]["ForecastStr"] = RFX_Forecast_Desc(atoi(strarray[4].c_str()));
							}
							if (dSubType == sTypeTHBFloat)
							{
								sprintf(szData, "%.1f %c, %d %%, %.1f hPa",
									temp,
									tempsign,
									atoi(strarray[1].c_str()),
									atof(strarray[3].c_str())
								);
							}
							else
							{
								sprintf(szData, "%.1f %c, %d %%, %d hPa",
									temp,
									tempsign,
									atoi(strarray[1].c_str()),
									atoi(strarray[3].c_str())
								);
							}
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;

							_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
							uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
							if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
							{
								tstate = m_mainworker.m_trend_calculator[tID].m_state;
							}
							root["result"][ii]["trend"] = (int)tstate;
						}
					}
					else if (dType == pTypeTEMP_BARO)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() >= 3)
						{
							double tvalue = ConvertTemperature(atof(strarray[0].c_str()), tempsign);
							root["result"][ii]["Temp"] = tvalue;
							int forecast = atoi(strarray[2].c_str());
							root["result"][ii]["Forecast"] = forecast;
							root["result"][ii]["ForecastStr"] = BMP_Forecast_Desc(forecast);
							root["result"][ii]["Barometer"] = atof(strarray[1].c_str());

							sprintf(szData, "%.1f %c, %.1f hPa",
								tvalue,
								tempsign,
								atof(strarray[1].c_str())
							);
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;

							_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
							uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
							if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
							{
								tstate = m_mainworker.m_trend_calculator[tID].m_state;
							}
							root["result"][ii]["trend"] = (int)tstate;
						}
					}
					else if (dType == pTypeUV)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 2)
						{
							float UVI = static_cast<float>(atof(strarray[0].c_str()));
							root["result"][ii]["UVI"] = strarray[0];
							if (dSubType == sTypeUV3)
							{
								double tvalue = ConvertTemperature(atof(strarray[1].c_str()), tempsign);

								root["result"][ii]["Temp"] = tvalue;
								sprintf(szData, "%.1f UVI, %.1f&deg; %c", UVI, tvalue, tempsign);

								_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
								uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
								if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
								{
									tstate = m_mainworker.m_trend_calculator[tID].m_state;
								}
								root["result"][ii]["trend"] = (int)tstate;
							}
							else
							{
								sprintf(szData, "%.1f UVI", UVI);
							}
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if (dType == pTypeWIND)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 6)
						{
							root["result"][ii]["Direction"] = atof(strarray[0].c_str());
							root["result"][ii]["DirectionStr"] = strarray[1];

							if (dSubType != sTypeWIND5)
							{
								int intSpeed = atoi(strarray[2].c_str());
								if (m_sql.m_windunit != WINDUNIT_Beaufort)
								{
									sprintf(szTmp, "%.1f", float(intSpeed) * m_sql.m_windscale);
								}
								else
								{
									float windms = float(intSpeed) * 0.1f;
									sprintf(szTmp, "%d", MStoBeaufort(windms));
								}
								root["result"][ii]["Speed"] = szTmp;
							}

							{
								int intGust = atoi(strarray[3].c_str());
								if (m_sql.m_windunit != WINDUNIT_Beaufort)
								{
									sprintf(szTmp, "%.1f", float(intGust) *m_sql.m_windscale);
								}
								else
								{
									float gustms = float(intGust) * 0.1f;
									sprintf(szTmp, "%d", MStoBeaufort(gustms));
								}
								root["result"][ii]["Gust"] = szTmp;
							}
							if ((dSubType == sTypeWIND4) || (dSubType == sTypeWINDNoTemp))
							{
								if (dSubType == sTypeWIND4)
								{
									double tvalue = ConvertTemperature(atof(strarray[4].c_str()), tempsign);
									root["result"][ii]["Temp"] = tvalue;
								}
								double tvalue = ConvertTemperature(atof(strarray[5].c_str()), tempsign);
								root["result"][ii]["Chill"] = tvalue;

								_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
								uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
								if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
								{
									tstate = m_mainworker.m_trend_calculator[tID].m_state;
								}
								root["result"][ii]["trend"] = (int)tstate;
							}
							root["result"][ii]["Data"] = sValue;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if (dType == pTypeRAIN)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 2)
						{
							time_t now = mytime(NULL);
							struct tm ltime;
							localtime_r(&now, &ltime);
							char szDate[40];
							sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

							std::vector<std::vector<std::string> > result2;

							if (dSubType != sTypeRAINWU)
							{
								result2 = m_sql.safe_query(
									"SELECT MIN(Total), MAX(Total) FROM Rain WHERE (DeviceRowID='%q' AND Date>='%q')", sd[0].c_str(), szDate);
							}
							else
							{
								result2 = m_sql.safe_query(
									"SELECT Total, Total FROM Rain WHERE (DeviceRowID='%q' AND Date>='%q') ORDER BY ROWID DESC LIMIT 1", sd[0].c_str(), szDate);
							}
							if (!result2.empty())
							{
								double total_real = 0;
								float rate = 0;
								std::vector<std::string> sd2 = result2[0];
								if (dSubType != sTypeRAINWU)
								{
									double total_min = atof(sd2[0].c_str());
									double total_max = atof(strarray[1].c_str());
									total_real = total_max - total_min;
								}
								else
								{
									total_real = atof(sd2[1].c_str());
								}
								total_real *= AddjMulti;
								rate = (static_cast<float>(atof(strarray[0].c_str())) / 100.0f)*float(AddjMulti);

								sprintf(szTmp, "%.1f", total_real);
								root["result"][ii]["Rain"] = szTmp;
								sprintf(szTmp, "%g", rate);
								root["result"][ii]["RainRate"] = szTmp;
								root["result"][ii]["Data"] = sValue;
								root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							}
							else
							{
								root["result"][ii]["Rain"] = "0";
								root["result"][ii]["RainRate"] = "0";
								root["result"][ii]["Data"] = "0";
								root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							}
						}
					}
					else if (dType == pTypeRFXMeter)
					{
						std::string ValueQuantity = options["ValueQuantity"];
						std::string ValueUnits = options["ValueUnits"];

						if (ValueQuantity.empty()) {
							ValueQuantity.assign("Count");
						}
						if (ValueUnits.empty()) {
							ValueUnits.assign("");
						}

						time_t now = mytime(NULL);
						struct tm ltime;
						localtime_r(&now, &ltime);
						char szDate[40];
						sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

						std::vector<std::vector<std::string> > result2;
						strcpy(szTmp, "0");
						result2 = m_sql.safe_query("SELECT MIN(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')", sd[0].c_str(), szDate);
						if (!result2.empty())
						{
							std::vector<std::string> sd2 = result2[0];

							uint64_t total_min = std::stoull(sd2[0]);
							uint64_t total_max = std::stoull(sValue);
							uint64_t total_real = total_max - total_min;
							sprintf(szTmp, "%" PRIu64, total_real);

							float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));
							float musage = 0.0f;
							switch (metertype)
							{
							case MTYPE_ENERGY:
							case MTYPE_ENERGY_GENERATED:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f kWh", musage);
								break;
							case MTYPE_GAS:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f m3", musage);
								break;
							case MTYPE_WATER:
								musage = float(total_real) / (divider / 1000.0f);
								sprintf(szTmp, "%d Liter", round(musage));
								break;
							case MTYPE_COUNTER:
								sprintf(szTmp, "%" PRIu64, total_real);
								if (!ValueUnits.empty())
								{
									strcat(szTmp, " ");
									strcat(szTmp, ValueUnits.c_str());
								}
								break;
							default:
								strcpy(szTmp, "?");
								break;
							}
						}
						root["result"][ii]["CounterToday"] = szTmp;

						root["result"][ii]["SwitchTypeVal"] = metertype;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						root["result"][ii]["ValueQuantity"] = "";
						root["result"][ii]["ValueUnits"] = "";

						double meteroffset = AddjValue;
						float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

						double dvalue = static_cast<double>(atof(sValue.c_str()));

						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							sprintf(szTmp, "%.3f kWh", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_GAS:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_WATER:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%g %s", meteroffset + dvalue, ValueUnits.c_str());
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						default:
							root["result"][ii]["Data"] = "?";
							root["result"][ii]["Counter"] = "?";
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						}
					}
					else if ((dType == pTypeGeneral) && (dSubType == sTypeCounterIncremental))
					{
						std::string ValueQuantity = options["ValueQuantity"];
						std::string ValueUnits = options["ValueUnits"];
						if (ValueQuantity.empty()) {
							ValueQuantity.assign("Count");
						}
						if (ValueUnits.empty()) {
							ValueUnits.assign("");
						}
						float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

						time_t now = mytime(NULL);
						struct tm ltime;
						localtime_r(&now, &ltime);
						char szDate[40];
						sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

						std::vector<std::vector<std::string> > result2;
						strcpy(szTmp, "0");
						result2 = m_sql.safe_query("SELECT MIN(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')", sd[0].c_str(), szDate);
						if (!result2.empty())
						{
							std::vector<std::string> sd2 = result2[0];

							uint64_t total_min = std::stoull(sd2[0]);
							uint64_t total_max = std::stoull(sValue);
							uint64_t total_real = total_max - total_min;
							sprintf(szTmp, "%" PRIu64, total_real);

							float musage = 0;
							switch (metertype)
							{
							case MTYPE_ENERGY:
							case MTYPE_ENERGY_GENERATED:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f kWh", musage);
								break;
							case MTYPE_GAS:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f m3", musage);
								break;
							case MTYPE_WATER:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f m3", musage);
								break;
							case MTYPE_COUNTER:
								sprintf(szTmp, "%" PRIu64, total_real);
								if (!ValueUnits.empty())
								{
									strcat(szTmp, " ");
									strcat(szTmp, ValueUnits.c_str());
								}
								break;
							default:
								strcpy(szTmp, "0");
								break;
							}
						}
						root["result"][ii]["Counter"] = sValue;
						root["result"][ii]["CounterToday"] = szTmp;
						root["result"][ii]["SwitchTypeVal"] = metertype;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						root["result"][ii]["TypeImg"] = "counter";
						root["result"][ii]["ValueQuantity"] = "";
						root["result"][ii]["ValueUnits"] = "";
						double dvalue = static_cast<double>(atof(sValue.c_str()));
						double meteroffset = AddjValue;

						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							sprintf(szTmp, "%.3f kWh", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_GAS:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_WATER:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%" PRIu64 " %s", static_cast<uint64_t>(meteroffset + dvalue), ValueUnits.c_str());
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						default:
							root["result"][ii]["Data"] = "?";
							root["result"][ii]["Counter"] = "?";
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						}
					}
					else if ((dType == pTypeGeneral) && (dSubType == sTypeManagedCounter))
					{
						std::string ValueQuantity = options["ValueQuantity"];
						std::string ValueUnits = options["ValueUnits"];
						if (ValueQuantity.empty()) {
							ValueQuantity.assign("Count");
						}
						if (ValueUnits.empty()) {
							ValueUnits.assign("");
						}
						float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

						std::vector<std::string> splitresults;
						StringSplit(sValue, ";", splitresults);
						double dvalue;
						if (splitresults.size() < 2) {
							dvalue = static_cast<double>(atof(sValue.c_str()));
						}
						else {
							dvalue = static_cast<double>(atof(splitresults[1].c_str()));
							if (dvalue < 0.0) {
								dvalue = static_cast<double>(atof(splitresults[0].c_str()));
							}
						}
						root["result"][ii]["Data"] = root["result"][ii]["Counter"];

						root["result"][ii]["SwitchTypeVal"] = metertype;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						root["result"][ii]["TypeImg"] = "counter";
						root["result"][ii]["ValueQuantity"] = "";
						root["result"][ii]["ValueUnits"] = "";
						root["result"][ii]["ShowNotifications"] = false;
						double meteroffset = AddjValue;

						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							sprintf(szTmp, "%.3f kWh", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_GAS:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_WATER:
							sprintf(szTmp, "%.3f m3", meteroffset + (dvalue / divider));
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%g %s", meteroffset + dvalue, ValueUnits.c_str());
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Counter"] = szTmp;
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						default:
							root["result"][ii]["Data"] = "?";
							root["result"][ii]["Counter"] = "?";
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						}
					}
					else if (dType == pTypeYouLess)
					{
						std::string ValueQuantity = options["ValueQuantity"];
						std::string ValueUnits = options["ValueUnits"];
						float musage = 0;
						if (ValueQuantity.empty()) {
							ValueQuantity.assign("Count");
						}
						if (ValueUnits.empty()) {
							ValueUnits.assign("");
						}
						float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

						time_t now = mytime(NULL);
						struct tm ltime;
						localtime_r(&now, &ltime);
						char szDate[40];
						sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

						std::vector<std::vector<std::string> > result2;
						strcpy(szTmp, "0");
						result2 = m_sql.safe_query("SELECT MIN(Value), MAX(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')", sd[0].c_str(), szDate);
						if (!result2.empty())
						{
							std::vector<std::string> sd2 = result2[0];

							unsigned long long total_min = std::strtoull(sd2[0].c_str(), nullptr, 10);
							unsigned long long total_max = std::strtoull(sd2[1].c_str(), nullptr, 10);
							unsigned long long total_real;

							total_real = total_max - total_min;
							sprintf(szTmp, "%llu", total_real);

							musage = 0;
							switch (metertype)
							{
							case MTYPE_ENERGY:
							case MTYPE_ENERGY_GENERATED:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f kWh", musage);
								break;
							case MTYPE_GAS:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f m3", musage);
								break;
							case MTYPE_WATER:
								musage = float(total_real) / divider;
								sprintf(szTmp, "%.3f m3", musage);
								break;
							case MTYPE_COUNTER:
								sprintf(szTmp, "%llu %s", total_real, ValueUnits.c_str());
								break;
							default:
								strcpy(szTmp, "0");
								break;
							}
						}
						root["result"][ii]["CounterToday"] = szTmp;


						std::vector<std::string> splitresults;
						StringSplit(sValue, ";", splitresults);
						if (splitresults.size() < 2)
							continue;

						unsigned long long total_actual = std::strtoull(splitresults[0].c_str(), nullptr, 10);
						musage = 0;
						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							musage = float(total_actual) / divider;
							sprintf(szTmp, "%.03f", musage);
							break;
						case MTYPE_GAS:
						case MTYPE_WATER:
							musage = float(total_actual) / divider;
							sprintf(szTmp, "%.03f", musage);
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%llu", total_actual);
							break;
						default:
							strcpy(szTmp, "0");
							break;
						}
						root["result"][ii]["Counter"] = szTmp;

						root["result"][ii]["SwitchTypeVal"] = metertype;

						unsigned long long acounter = std::strtoull(sValue.c_str(), nullptr, 10);
						musage = 0;
						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							musage = float(acounter) / divider;
							sprintf(szTmp, "%.3f kWh %s Watt", musage, splitresults[1].c_str());
							break;
						case MTYPE_GAS:
							musage = float(acounter) / divider;
							sprintf(szTmp, "%.3f m3", musage);
							break;
						case MTYPE_WATER:
							musage = float(acounter) / divider;
							sprintf(szTmp, "%.3f m3", musage);
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%llu %s", acounter, ValueUnits.c_str());
							break;
						default:
							strcpy(szTmp, "0");
							break;
						}
						root["result"][ii]["Data"] = szTmp;
						root["result"][ii]["ValueQuantity"] = "";
						root["result"][ii]["ValueUnits"] = "";
						switch (metertype)
						{
						case MTYPE_ENERGY:
						case MTYPE_ENERGY_GENERATED:
							sprintf(szTmp, "%s Watt", splitresults[1].c_str());
							break;
						case MTYPE_GAS:
							sprintf(szTmp, "%s m3", splitresults[1].c_str());
							break;
						case MTYPE_WATER:
							sprintf(szTmp, "%s m3", splitresults[1].c_str());
							break;
						case MTYPE_COUNTER:
							sprintf(szTmp, "%s", splitresults[1].c_str());
							root["result"][ii]["ValueQuantity"] = ValueQuantity;
							root["result"][ii]["ValueUnits"] = ValueUnits;
							break;
						default:
							strcpy(szTmp, "0");
							break;
						}

						root["result"][ii]["Usage"] = szTmp;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
					}
					else if (dType == pTypeP1Power)
					{
						std::vector<std::string> splitresults;
						StringSplit(sValue, ";", splitresults);
						if (splitresults.size() != 6)
						{
							root["result"][ii]["SwitchTypeVal"] = MTYPE_ENERGY;
							root["result"][ii]["Counter"] = "0";
							root["result"][ii]["CounterDeliv"] = "0";
							root["result"][ii]["Usage"] = "Invalid";
							root["result"][ii]["UsageDeliv"] = "Invalid";
							root["result"][ii]["Data"] = "Invalid!: " + sValue;
							root["result"][ii]["HaveTimeout"] = true;
							root["result"][ii]["CounterToday"] = "Invalid";
							root["result"][ii]["CounterDelivToday"] = "Invalid";
						}
						else
						{
							float EnergyDivider = 1000.0f;
							int tValue;
							if (m_sql.GetPreferencesVar("MeterDividerEnergy", tValue))
							{
								EnergyDivider = float(tValue);
							}

							unsigned long long powerusage1 = std::strtoull(splitresults[0].c_str(), nullptr, 10);
							unsigned long long powerusage2 = std::strtoull(splitresults[1].c_str(), nullptr, 10);
							unsigned long long powerdeliv1 = std::strtoull(splitresults[2].c_str(), nullptr, 10);
							unsigned long long powerdeliv2 = std::strtoull(splitresults[3].c_str(), nullptr, 10);
							unsigned long long usagecurrent = std::strtoull(splitresults[4].c_str(), nullptr, 10);
							unsigned long long delivcurrent = std::strtoull(splitresults[5].c_str(), nullptr, 10);

							powerdeliv1 = (powerdeliv1 < 10) ? 0 : powerdeliv1;
							powerdeliv2 = (powerdeliv2 < 10) ? 0 : powerdeliv2;

							unsigned long long powerusage = powerusage1 + powerusage2;
							unsigned long long powerdeliv = powerdeliv1 + powerdeliv2;
							if (powerdeliv < 2)
								powerdeliv = 0;

							double musage = 0;

							root["result"][ii]["SwitchTypeVal"] = MTYPE_ENERGY;
							musage = double(powerusage) / EnergyDivider;
							sprintf(szTmp, "%.03f", musage);
							root["result"][ii]["Counter"] = szTmp;
							musage = double(powerdeliv) / EnergyDivider;
							sprintf(szTmp, "%.03f", musage);
							root["result"][ii]["CounterDeliv"] = szTmp;

							if (bHaveTimeout)
							{
								usagecurrent = 0;
								delivcurrent = 0;
							}
							sprintf(szTmp, "%llu Watt", usagecurrent);
							root["result"][ii]["Usage"] = szTmp;
							sprintf(szTmp, "%llu Watt", delivcurrent);
							root["result"][ii]["UsageDeliv"] = szTmp;
							root["result"][ii]["Data"] = sValue;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;

							time_t now = mytime(NULL);
							struct tm ltime;
							localtime_r(&now, &ltime);
							char szDate[40];
							sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

							std::vector<std::vector<std::string> > result2;
							strcpy(szTmp, "0");
							result2 = m_sql.safe_query("SELECT MIN(Value1), MIN(Value2), MIN(Value5), MIN(Value6) FROM MultiMeter WHERE (DeviceRowID='%q' AND Date>='%q')",
								sd[0].c_str(), szDate);
							if (!result2.empty())
							{
								std::vector<std::string> sd2 = result2[0];

								unsigned long long total_min_usage_1 = std::strtoull(sd2[0].c_str(), nullptr, 10);
								unsigned long long total_min_deliv_1 = std::strtoull(sd2[1].c_str(), nullptr, 10);
								unsigned long long total_min_usage_2 = std::strtoull(sd2[2].c_str(), nullptr, 10);
								unsigned long long total_min_deliv_2 = std::strtoull(sd2[3].c_str(), nullptr, 10);
								unsigned long long total_real_usage, total_real_deliv;

								total_real_usage = powerusage - (total_min_usage_1 + total_min_usage_2);
								total_real_deliv = powerdeliv - (total_min_deliv_1 + total_min_deliv_2);

								musage = double(total_real_usage) / EnergyDivider;
								sprintf(szTmp, "%.3f kWh", musage);
								root["result"][ii]["CounterToday"] = szTmp;
								musage = double(total_real_deliv) / EnergyDivider;
								sprintf(szTmp, "%.3f kWh", musage);
								root["result"][ii]["CounterDelivToday"] = szTmp;
							}
							else
							{
								sprintf(szTmp, "%.3f kWh", 0.0f);
								root["result"][ii]["CounterToday"] = szTmp;
								root["result"][ii]["CounterDelivToday"] = szTmp;
							}
						}
					}
					else if (dType == pTypeP1Gas)
					{
						root["result"][ii]["SwitchTypeVal"] = MTYPE_GAS;

						time_t now = mytime(NULL);
						struct tm ltime;
						localtime_r(&now, &ltime);
						char szDate[40];
						sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

						std::vector<std::vector<std::string> > result2;

						float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

						strcpy(szTmp, "0");
						result2 = m_sql.safe_query("SELECT MIN(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')",
							sd[0].c_str(), szDate);
						if (!result2.empty())
						{
							std::vector<std::string> sd2 = result2[0];

							uint64_t total_min_gas = std::stoull(sd2[0]);
							uint64_t gasactual = std::stoull(sValue);
							uint64_t total_real_gas = gasactual - total_min_gas;

							double musage = double(gasactual) / divider;
							sprintf(szTmp, "%.03f", musage);
							root["result"][ii]["Counter"] = szTmp;
							musage = double(total_real_gas) / divider;
							sprintf(szTmp, "%.03f m3", musage);
							root["result"][ii]["CounterToday"] = szTmp;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							sprintf(szTmp, "%.03f", atof(sValue.c_str()) / divider);
							root["result"][ii]["Data"] = szTmp;
						}
						else
						{
							sprintf(szTmp, "%.03f", 0.0f);
							root["result"][ii]["Counter"] = szTmp;
							sprintf(szTmp, "%.03f m3", 0.0f);
							root["result"][ii]["CounterToday"] = szTmp;
							sprintf(szTmp, "%.03f", atof(sValue.c_str()) / divider);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if (dType == pTypeCURRENT)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 3)
						{
							int displaytype = 0;
							int voltage = 230;
							m_sql.GetPreferencesVar("CM113DisplayType", displaytype);
							m_sql.GetPreferencesVar("ElectricVoltage", voltage);

							double val1 = atof(strarray[0].c_str());
							double val2 = atof(strarray[1].c_str());
							double val3 = atof(strarray[2].c_str());

							if (displaytype == 0)
							{
								if ((val2 == 0) && (val3 == 0))
									sprintf(szData, "%.1f A", val1);
								else
									sprintf(szData, "%.1f A, %.1f A, %.1f A", val1, val2, val3);
							}
							else
							{
								if ((val2 == 0) && (val3 == 0))
									sprintf(szData, "%d Watt", int(val1*voltage));
								else
									sprintf(szData, "%d Watt, %d Watt, %d Watt", int(val1*voltage), int(val2*voltage), int(val3*voltage));
							}
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["displaytype"] = displaytype;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if (dType == pTypeCURRENTENERGY)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 4)
						{
							int displaytype = 0;
							int voltage = 230;
							m_sql.GetPreferencesVar("CM113DisplayType", displaytype);
							m_sql.GetPreferencesVar("ElectricVoltage", voltage);

							double total = atof(strarray[3].c_str());
							if (displaytype == 0)
							{
								sprintf(szData, "%.1f A, %.1f A, %.1f A", atof(strarray[0].c_str()), atof(strarray[1].c_str()), atof(strarray[2].c_str()));
							}
							else
							{
								sprintf(szData, "%d Watt, %d Watt, %d Watt", int(atof(strarray[0].c_str())*voltage), int(atof(strarray[1].c_str())*voltage), int(atof(strarray[2].c_str())*voltage));
							}
							if (total > 0)
							{
								sprintf(szTmp, ", Total: %.3f kWh", total / 1000.0f);
								strcat(szData, szTmp);
							}
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["displaytype"] = displaytype;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
					else if (
						((dType == pTypeENERGY) || (dType == pTypePOWER)) ||
						((dType == pTypeGeneral) && (dSubType == sTypeKwh))
						)
					{
						std::vector<std::string> strarray;
						StringSplit(sValue, ";", strarray);
						if (strarray.size() == 2)
						{
							double total = atof(strarray[1].c_str()) / 1000;

							time_t now = mytime(NULL);
							struct tm ltime;
							localtime_r(&now, &ltime);
							char szDate[40];
							sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

							std::vector<std::vector<std::string> > result2;
							strcpy(szTmp, "0");
							result2 = m_sql.safe_query("SELECT MIN(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')",
								sd[0].c_str(), szDate);
							if (!result2.empty())
							{
								float divider = m_sql.GetCounterDivider(int(metertype), int(dType), float(AddjValue2));

								std::vector<std::string> sd2 = result2[0];
								double minimum = atof(sd2[0].c_str()) / divider;

								sprintf(szData, "%.3f kWh", total);
								root["result"][ii]["Data"] = szData;
								if ((dType == pTypeENERGY) || (dType == pTypePOWER))
								{
									sprintf(szData, "%ld Watt", atol(strarray[0].c_str()));
								}
								else
								{
									sprintf(szData, "%g Watt", atof(strarray[0].c_str()));
								}
								root["result"][ii]["Usage"] = szData;
								root["result"][ii]["HaveTimeout"] = bHaveTimeout;
								sprintf(szTmp, "%.3f kWh", total - minimum);
								root["result"][ii]["CounterToday"] = szTmp;
							}
							else
							{
								sprintf(szData, "%.3f kWh", total);
								root["result"][ii]["Data"] = szData;
								if ((dType == pTypeENERGY) || (dType == pTypePOWER))
								{
									sprintf(szData, "%ld Watt", atol(strarray[0].c_str()));
								}
								else
								{
									sprintf(szData, "%g Watt", atof(strarray[0].c_str()));
								}
								root["result"][ii]["Usage"] = szData;
								root["result"][ii]["HaveTimeout"] = bHaveTimeout;
								sprintf(szTmp, "%d kWh", 0);
								root["result"][ii]["CounterToday"] = szTmp;
							}
							root["result"][ii]["TypeImg"] = "current";
							root["result"][ii]["SwitchTypeVal"] = switchtype; //MTYPE_ENERGY
							root["result"][ii]["EnergyMeterMode"] = options["EnergyMeterMode"];  //for alternate Energy Reading
						}
					}
					else if (dType == pTypeAirQuality)
					{
						if (bHaveTimeout)
							nValue = 0;
						sprintf(szTmp, "%d ppm", nValue);
						root["result"][ii]["Data"] = szTmp;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						int airquality = nValue;
						if (airquality < 700)
							root["result"][ii]["Quality"] = "Excellent";
						else if (airquality < 900)
							root["result"][ii]["Quality"] = "Good";
						else if (airquality < 1100)
							root["result"][ii]["Quality"] = "Fair";
						else if (airquality < 1600)
							root["result"][ii]["Quality"] = "Mediocre";
						else
							root["result"][ii]["Quality"] = "Bad";
					}
					else if (dType == pTypeThermostat)
					{
						if (dSubType == sTypeThermSetpoint)
						{
							bHasTimers = m_sql.HasTimers(sd[0]);

							double tempCelcius = atof(sValue.c_str());
							double temp = ConvertTemperature(tempCelcius, tempsign);

							sprintf(szTmp, "%.1f", temp);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["SetPoint"] = szTmp;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["TypeImg"] = "override_mini";
						}
					}
					else if (dType == pTypeRadiator1)
					{
						if (dSubType == sTypeSmartwares)
						{
							bHasTimers = m_sql.HasTimers(sd[0]);

							double tempCelcius = atof(sValue.c_str());
							double temp = ConvertTemperature(tempCelcius, tempsign);

							sprintf(szTmp, "%.1f", temp);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["SetPoint"] = szTmp;
							root["result"][ii]["HaveTimeout"] = false; //this device does not provide feedback, so no timeout!
							root["result"][ii]["TypeImg"] = "override_mini";
						}
					}
					else if (dType == pTypeGeneral)
					{
						if (dSubType == sTypeVisibility)
						{
							float vis = static_cast<float>(atof(sValue.c_str()));
							if (metertype == 0)
							{
								sprintf(szTmp, "%.1f km", vis);
							}
							else
							{
								sprintf(szTmp, "%.1f mi", vis*0.6214f);
							}
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Visibility"] = atof(sValue.c_str());
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["TypeImg"] = "visibility";
							root["result"][ii]["SwitchTypeVal"] = metertype;
						}
						else if (dSubType == sTypeDistance)
						{
							float vis = static_cast<float>(atof(sValue.c_str()));
							if (metertype == 0)
							{
								sprintf(szTmp, "%.1f cm", vis);
							}
							else
							{
								sprintf(szTmp, "%.1f in", vis*0.6214f);
							}
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["TypeImg"] = "visibility";
							root["result"][ii]["SwitchTypeVal"] = metertype;
						}
						else if (dSubType == sTypeSolarRadiation)
						{
							float radiation = static_cast<float>(atof(sValue.c_str()));
							sprintf(szTmp, "%.1f Watt/m2", radiation);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Radiation"] = atof(sValue.c_str());
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["TypeImg"] = "radiation";
							root["result"][ii]["SwitchTypeVal"] = metertype;
						}
						else if (dSubType == sTypeSoilMoisture)
						{
							sprintf(szTmp, "%d cb", nValue);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["Desc"] = Get_Moisture_Desc(nValue);
							root["result"][ii]["TypeImg"] = "moisture";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["SwitchTypeVal"] = metertype;
						}
						else if (dSubType == sTypeLeafWetness)
						{
							sprintf(szTmp, "%d", nValue);
							root["result"][ii]["Data"] = szTmp;
							root["result"][ii]["TypeImg"] = "leaf";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["SwitchTypeVal"] = metertype;
						}
						else if (dSubType == sTypeSystemTemp)
						{
							double tvalue = ConvertTemperature(atof(sValue.c_str()), tempsign);
							root["result"][ii]["Temp"] = tvalue;
							sprintf(szData, "%.1f %c", tvalue, tempsign);
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Image"] = "Computer";
							root["result"][ii]["TypeImg"] = "temperature";
							root["result"][ii]["Type"] = "temperature";
							_tTrendCalculator::_eTendencyType tstate = _tTrendCalculator::_eTendencyType::TENDENCY_UNKNOWN;
							uint64_t tID = ((uint64_t)(hardwareID & 0x7FFFFFFF) << 32) | (devIdx & 0x7FFFFFFF);
							if (m_mainworker.m_trend_calculator.find(tID) != m_mainworker.m_trend_calculator.end())
							{
								tstate = m_mainworker.m_trend_calculator[tID].m_state;
							}
							root["result"][ii]["trend"] = (int)tstate;
						}
						else if (dSubType == sTypePercentage)
						{
							sprintf(szData, "%g%%", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Image"] = "Computer";
							root["result"][ii]["TypeImg"] = "hardware";
						}
						else if (dSubType == sTypeWaterflow)
						{
							sprintf(szData, "%g l/min", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Image"] = "Moisture";
							root["result"][ii]["TypeImg"] = "moisture";
						}
						else if (dSubType == sTypeCustom)
						{
							std::string szAxesLabel = "";
							int SensorType = 1;
							std::vector<std::string> sResults;
							StringSplit(sOptions, ";", sResults);

							if (sResults.size() == 2)
							{
								SensorType = atoi(sResults[0].c_str());
								szAxesLabel = sResults[1];
							}
							sprintf(szData, "%g %s", atof(sValue.c_str()), szAxesLabel.c_str());
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["SensorType"] = SensorType;
							root["result"][ii]["SensorUnit"] = szAxesLabel;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;

							std::string IconFile = "Custom";
							if (CustomImage != 0)
							{
								std::map<int, int>::const_iterator ittIcon = m_custom_light_icons_lookup.find(CustomImage);
								if (ittIcon != m_custom_light_icons_lookup.end())
								{
									IconFile = m_custom_light_icons[ittIcon->second].RootFile;
								}
							}
							root["result"][ii]["Image"] = IconFile;
							root["result"][ii]["TypeImg"] = IconFile;
						}
						else if (dSubType == sTypeFan)
						{
							sprintf(szData, "%d RPM", atoi(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Image"] = "Fan";
							root["result"][ii]["TypeImg"] = "Fan";
						}
						else if (dSubType == sTypeSoundLevel)
						{
							sprintf(szData, "%d dB", atoi(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "Speaker";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
						else if (dSubType == sTypeVoltage)
						{
							sprintf(szData, "%g V", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "current";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Voltage"] = atof(sValue.c_str());
						}
						else if (dSubType == sTypeCurrent)
						{
							sprintf(szData, "%g A", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "current";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Current"] = atof(sValue.c_str());
						}
						else if (dSubType == sTypeTextStatus)
						{
							root["result"][ii]["Data"] = sValue;
							root["result"][ii]["TypeImg"] = "text";
							root["result"][ii]["HaveTimeout"] = false;
							root["result"][ii]["ShowNotifications"] = false;
						}
						else if (dSubType == sTypeAlert)
						{
							if (nValue > 4)
								nValue = 4;
							sprintf(szData, "Level: %d", nValue);
							root["result"][ii]["Data"] = szData;
							if (!sValue.empty())
								root["result"][ii]["Data"] = sValue;
							else
								root["result"][ii]["Data"] = Get_Alert_Desc(nValue);
							root["result"][ii]["TypeImg"] = "Alert";
							root["result"][ii]["Level"] = nValue;
							root["result"][ii]["HaveTimeout"] = false;
						}
						else if (dSubType == sTypePressure)
						{
							sprintf(szData, "%.1f Bar", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "gauge";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["Pressure"] = atof(sValue.c_str());
						}
						else if (dSubType == sTypeBaro)
						{
							std::vector<std::string> tstrarray;
							StringSplit(sValue, ";", tstrarray);
							if (tstrarray.empty())
								continue;
							sprintf(szData, "%g hPa", atof(tstrarray[0].c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "gauge";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							if (tstrarray.size() > 1)
							{
								root["result"][ii]["Barometer"] = atof(tstrarray[0].c_str());
								int forecast = atoi(tstrarray[1].c_str());
								root["result"][ii]["Forecast"] = forecast;
								root["result"][ii]["ForecastStr"] = BMP_Forecast_Desc(forecast);
							}
						}
						else if (dSubType == sTypeZWaveClock)
						{
							std::vector<std::string> tstrarray;
							StringSplit(sValue, ";", tstrarray);
							int day = 0;
							int hour = 0;
							int minute = 0;
							if (tstrarray.size() == 3)
							{
								day = atoi(tstrarray[0].c_str());
								hour = atoi(tstrarray[1].c_str());
								minute = atoi(tstrarray[2].c_str());
							}
							sprintf(szData, "%s %02d:%02d", ZWave_Clock_Days(day), hour, minute);
							root["result"][ii]["DayTime"] = sValue;
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["TypeImg"] = "clock";
						}
						else if (dSubType == sTypeZWaveThermostatMode)
						{
							strcpy(szData, "");
							root["result"][ii]["Mode"] = nValue;
							root["result"][ii]["TypeImg"] = "mode";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							std::string modes = "";
#ifdef WITH_OPENZWAVE
							if (pHardware)
							{
								if (pHardware->HwdType == HTYPE_OpenZWave)
								{
									COpenZWave *pZWave = reinterpret_cast<COpenZWave*>(pHardware);
									unsigned long ID;
									std::stringstream s_strid;
									s_strid << std::hex << sd[1];
									s_strid >> ID;
									std::vector<std::string> vmodes = pZWave->GetSupportedThermostatModes(ID);
									int smode = 0;
									char szTmp[200];
									for (const auto & itt : vmodes)
									{
										sprintf(szTmp, "%d;%s;", smode, itt.c_str());
										modes += szTmp;
										smode++;
									}

									if (!vmodes.empty())
									{
										if (nValue < (int)vmodes.size())
										{
											sprintf(szData, "%s", vmodes[nValue].c_str());
										}
									}
								}
							}
#endif
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["Modes"] = modes;
						}
						else if (dSubType == sTypeZWaveThermostatFanMode)
						{
							sprintf(szData, "%s", ZWave_Thermostat_Fan_Modes[nValue]);
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["Mode"] = nValue;
							root["result"][ii]["TypeImg"] = "mode";
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							bool bAddedSupportedModes = false;
							std::string modes = "";
#ifdef WITH_OPENZWAVE
							if (pHardware)
							{
								if (pHardware->HwdType == HTYPE_OpenZWave)
								{
									COpenZWave *pZWave = reinterpret_cast<COpenZWave*>(pHardware);
									unsigned long ID;
									std::stringstream s_strid;
									s_strid << std::hex << sd[1];
									s_strid >> ID;
									modes = pZWave->GetSupportedThermostatFanModes(ID);
									bAddedSupportedModes = !modes.empty();
								}
							}
#endif
							if (!bAddedSupportedModes)
							{
								int smode = 0;
								while (ZWave_Thermostat_Fan_Modes[smode] != NULL)
								{
									sprintf(szTmp, "%d;%s;", smode, ZWave_Thermostat_Fan_Modes[smode]);
									modes += szTmp;
									smode++;
								}
							}
							root["result"][ii]["Modes"] = modes;
						}
						else if (dSubType == sTypeZWaveAlarm)
						{
							sprintf(szData, "Event: 0x%02X (%d)", nValue, nValue);
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["TypeImg"] = "Alert";
							root["result"][ii]["Level"] = nValue;
							root["result"][ii]["HaveTimeout"] = false;
						}
					}
					else if (dType == pTypeLux)
					{
						sprintf(szTmp, "%.0f Lux", atof(sValue.c_str()));
						root["result"][ii]["Data"] = szTmp;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
					}
					else if (dType == pTypeWEIGHT)
					{
						sprintf(szTmp, "%g %s", m_sql.m_weightscale * atof(sValue.c_str()), m_sql.m_weightsign.c_str());
						root["result"][ii]["Data"] = szTmp;
						root["result"][ii]["HaveTimeout"] = false;
					}
					else if (dType == pTypeUsage)
					{
						if (dSubType == sTypeElectric)
						{
							sprintf(szData, "%g Watt", atof(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
						}
						else
						{
							root["result"][ii]["Data"] = sValue;
						}
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
					}
					else if (dType == pTypeRFXSensor)
					{
						switch (dSubType)
						{
						case sTypeRFXSensorAD:
							sprintf(szData, "%d mV", atoi(sValue.c_str()));
							root["result"][ii]["TypeImg"] = "current";
							break;
						case sTypeRFXSensorVolt:
							sprintf(szData, "%d mV", atoi(sValue.c_str()));
							root["result"][ii]["TypeImg"] = "current";
							break;
						}
						root["result"][ii]["Data"] = szData;
						root["result"][ii]["HaveTimeout"] = bHaveTimeout;
					}
					else if (dType == pTypeRego6XXValue)
					{
						switch (dSubType)
						{
						case sTypeRego6XXStatus:
						{
							std::string lstatus = "On";

							if (atoi(sValue.c_str()) == 0)
							{
								lstatus = "Off";
							}
							root["result"][ii]["Status"] = lstatus;
							root["result"][ii]["HaveDimmer"] = false;
							root["result"][ii]["MaxDimLevel"] = 0;
							root["result"][ii]["HaveGroupCmd"] = false;
							root["result"][ii]["TypeImg"] = "utility";
							root["result"][ii]["SwitchTypeVal"] = STYPE_OnOff;
							root["result"][ii]["SwitchType"] = Switch_Type_Desc(STYPE_OnOff);
							sprintf(szData, "%d", atoi(sValue.c_str()));
							root["result"][ii]["Data"] = szData;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
							root["result"][ii]["StrParam1"] = strParam1;
							root["result"][ii]["StrParam2"] = strParam2;
							root["result"][ii]["Protected"] = (iProtected != 0);

							if (CustomImage < static_cast<int>(m_custom_light_icons.size()))
								root["result"][ii]["Image"] = m_custom_light_icons[CustomImage].RootFile;
							else
								root["result"][ii]["Image"] = "Light";

							uint64_t camIDX = m_mainworker.m_cameras.IsDevSceneInCamera(0, sd[0]);
							root["result"][ii]["UsedByCamera"] = (camIDX != 0) ? true : false;
							if (camIDX != 0) {
								std::stringstream scidx;
								scidx << camIDX;
								root["result"][ii]["CameraIdx"] = scidx.str();
							}

							root["result"][ii]["Level"] = 0;
							root["result"][ii]["LevelInt"] = atoi(sValue.c_str());
						}
						break;
						case sTypeRego6XXCounter:
						{
							time_t now = mytime(NULL);
							struct tm ltime;
							localtime_r(&now, &ltime);
							char szDate[40];
							sprintf(szDate, "%04d-%02d-%02d", ltime.tm_year + 1900, ltime.tm_mon + 1, ltime.tm_mday);

							std::vector<std::vector<std::string> > result2;
							strcpy(szTmp, "0");
							result2 = m_sql.safe_query("SELECT MIN(Value), MAX(Value) FROM Meter WHERE (DeviceRowID='%q' AND Date>='%q')",
								sd[0].c_str(), szDate);
							if (!result2.empty())
							{
								std::vector<std::string> sd2 = result2[0];

								unsigned long long total_min = std::strtoull(sd2[0].c_str(), nullptr, 10);
								unsigned long long total_max = std::strtoull(sd2[1].c_str(), nullptr, 10);
								unsigned long long total_real;

								total_real = total_max - total_min;
								sprintf(szTmp, "%llu", total_real);
							}
							root["result"][ii]["SwitchTypeVal"] = MTYPE_COUNTER;
							root["result"][ii]["Counter"] = sValue;
							root["result"][ii]["CounterToday"] = szTmp;
							root["result"][ii]["Data"] = sValue;
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
						break;
						}
					}
#ifdef ENABLE_PYTHON
					if (pHardware != NULL)
					{
						if (pHardware->HwdType == HTYPE_PythonPlugin)
						{
							Plugins::CPlugin *pPlugin = (Plugins::CPlugin*)pHardware;
							bHaveTimeout = pPlugin->HasNodeFailed(atoi(sd[2].c_str()));
							root["result"][ii]["HaveTimeout"] = bHaveTimeout;
						}
					}
#endif
					root["result"][ii]["Timers"] = (bHasTimers == true) ? "true" : "false";
					ii++;
				}
			}
		}
