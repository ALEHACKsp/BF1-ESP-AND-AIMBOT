#pragma once
#include "globals.h"

void SetupColors()
{
	Globals::ColorsForPicker1.push_back(ColorP("Box", g_Options.COLOR.TeamColor));
	Globals::ColorsForPicker1.push_back(ColorP("Name", g_Options.COLOR.TeamNameColor));
	Globals::ColorsForPicker1.push_back(ColorP("Distance", g_Options.COLOR.TeamDistanceColor));
	Globals::ColorsForPicker1.push_back(ColorP("Line ", g_Options.COLOR.TeamLineColor));
	Globals::ColorsForPicker1.push_back(ColorP("Filedbox ", g_Options.COLOR.TeamFiledboxColor));
	Globals::ColorsForPicker1.push_back(ColorP("Skeleton", g_Options.COLOR.TeamSkeleton));


	Globals::ColorsForPicker2.push_back(ColorP("Vis Box", g_Options.COLOR.OccludedEnemyColor));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Box", g_Options.COLOR.VisibleEnemyColor));
	Globals::ColorsForPicker2.push_back(ColorP("Vis Name", g_Options.COLOR.VisNameColor));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Name", g_Options.COLOR.UnVisNameColor));
	Globals::ColorsForPicker2.push_back(ColorP("Vis Distance", g_Options.COLOR.VisDistanceColor));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Distance", g_Options.COLOR.UnVisDistanceColor));
	Globals::ColorsForPicker2.push_back(ColorP("Vis Line", g_Options.COLOR.VisLineColor));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Line", g_Options.COLOR.UnVisLineColor));
	Globals::ColorsForPicker2.push_back(ColorP("Vis Filedbox", g_Options.COLOR.VisFiledboxColor));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Filedbox", g_Options.COLOR.UnVisFiledboxColor));
	Globals::ColorsForPicker2.push_back(ColorP("Vis Skeleton", g_Options.COLOR.VisSkeleton));
	Globals::ColorsForPicker2.push_back(ColorP("UnVis Skeleton", g_Options.COLOR.UnVisSkeleton));


	Globals::ColorsForPicker3.push_back(ColorP("Name", g_Options.COLOR.VehicleNameColor));
	Globals::ColorsForPicker3.push_back(ColorP("3DBOX", g_Options.COLOR.VehicleColor));

	Globals::ColorsForPicker4.push_back(ColorP("Team", g_Options.COLOR.RadarTeam));
	Globals::ColorsForPicker4.push_back(ColorP("Vis Enemy", g_Options.COLOR.VisRadarEnemy));
	Globals::ColorsForPicker4.push_back(ColorP("UnVis Enemy", g_Options.COLOR.UnVisRadarEnemy));
	Globals::ColorsForPicker4.push_back(ColorP("Vehicle", g_Options.COLOR.VehicleRadar));

	Globals::ColorsForPicker5.push_back(ColorP("Crosshair", g_Options.COLOR.CrossHair));

}