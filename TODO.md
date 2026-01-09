# To do list
In no particular order:
+ `scan` command:  This is what will eventually be the "normal" operating mode.  
  + On startup, if there is no CLI action in XX seconds, start the `scan`
    + a `scan` can be started by CLI at any time
  + Scan operation:
    + Continually scan through WiFi channels and BTLE looking for target devices and alert if something is seen
    + Log to local filesytem
    + Roll logs

+ Memory leak in BTLE scan
  + Allocate the BT object in `psram`, and clean up appropriately
