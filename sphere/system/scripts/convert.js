function ScreenX(mapx)
{
  return MapToScreenX(0, mapx);
}

function ScreenY(mapy)
{
  return MapToScreenY(0, mapy);
}

function TopX()
{
  if (ScreenX(GetCameraX()) >= GetScreenWidth() / 2)
  {
    return - (ScreenX(GetCameraX()) - GetCameraX());
  }
  else
    return 0;
}

function TopY()
{
  if (ScreenY(GetCameraY()) >= GetScreenHeight() / 2)
  {
    return - (ScreenY(GetCameraY()) - GetCameraY());
  }
  else
    return 0;
}

function MapX(screenx)
{
  return TopX() + MapToScreenX(0, screenx);
}

function MapY(screeny)
{
  return TopY() + MapToScreenY(0, screeny);
}