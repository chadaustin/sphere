function ScreenX(mapx)
{
	return mapx - (MapToScreenX(0, GetCameraX()) - GetScreenWidth() / 2);
}

function ScreenY(mapy)
{
	return mapy - (MapToScreenY(0, GetCameraY()) - GetScreenHeight() / 2);
}

function TopX()
{
	if (ScreenX(GetCameraX()) >= GetScreenWidth() / 2)
	{
		return -(ScreenX(GetCameraX()) - GetCameraX());
	}
	else
		return 0;
}

function TopY()
{
	if (ScreenY(GetCameraY()) >= GetScreenHeight() / 2)
	{
		return -(ScreenY(GetCameraY()) - GetCameraY());
	}
	else
		return 0;
}

function MapX(screenx)
{
	return screenx + TopX();
}

function MapY(screeny)
{
	return screeny + TopY();
}