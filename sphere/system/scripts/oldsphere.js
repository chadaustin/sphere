EvaluateSystemScript("colors.js");


function ClearScreen()
{
  ApplyColorMask(Black);
}


function Delay(time)
{
  var until = GetTime() + time;
  while (GetTime() < until) {
  }
}
