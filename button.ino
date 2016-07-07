uint8_t upButtonPushed()
{
  if ( digitalRead(upButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(upButtonPin) == LOW) return (1);
  }
  return (0);
}
uint8_t downButtonPushed()
{
  if ( digitalRead(downButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(downButtonPin) == LOW) return (1);
  }
  return (0);
}
uint8_t leftButtonPushed()
{
  if ( digitalRead(leftButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(leftButtonPin) == LOW) return (1);
  }
  return (0);
}
uint8_t rightButtonPushed()
{
  if ( digitalRead(rightButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(rightButtonPin) == LOW) return (1);
  }
  return (0);
}
uint8_t cancelButtonPushed()
{
  if ( digitalRead(cancelButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(cancelButtonPin) == LOW) return (1);
  }
  return (0);
}
uint8_t enterButtonPushed()
{
  if ( digitalRead(enterButtonPin) == LOW )
  {
    delay(100);
    if (digitalRead(enterButtonPin) == LOW) return (1);
  }
  return (0);
}
