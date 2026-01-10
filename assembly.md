# Some general notes on constructing the device
## PCB
The `./hardware` folder has a KiCAD project for the PCB.  This should be sufficient if you are using the components listed in the `README.md` file.  (Yeah, I'll make a BOM at some point).  KiCAD can be used to generate gerber files for a board house.  The original prototypes were done at [OSHPark](https://oshpark.com).  With standard service, the total was just under $20USD for 3 boards.  It took about 3 weeks for them to arrive.

The board has four 4mm through holes in the corners for mounting.  This drawing shows the dimensions of the board, locations of the mounting holes, and the center point of the two 8mm LEDs (all dimensions in millimeters):
![image](./hardware/FlockOff/pcbDimensions.png)

## Soldering
The board is designed so everything is through-hole for easy home soldering.  The only tricky componenents are the two LEDs; they are somewhat fine pitch, so be sure to use a fine point on the soldering iron and a bit of flux to prevent solder bridges.  Everything else is on 0.100" pitch (or the PTH diode).

When soldering the LEDs, be mindful of the flat edge of the base - align that side of the LED with the flat on the silkscreen of the PCB. Note that they are in opposite orientations!

## Case/Enclosure
The `./case` folder has an OpenSCAD file for a sample enclosure, with included `.stl` files.  Feel free to use that one, remix it, make your own, or done use one at all.

If you are using the included OpenSCAD file, look for `main` near the end of the file.  The file is the commplete enclosure, both top and bottom, along with a very rough representation of the PCB and GPS antenna for reference.  When working with the file, you can show/hide different parts by simply commenting/uncommenting lines in `main`:
```  
module main()
{
    // the whole shebang
    bottom(90, 42, 20);
    top(90, 42, 4, 20);
    pcb(-5, 0, 10);
    antenna();
    lanyardRing(42, 18, 12);
    //posts();
}
```  
For example, commenting everything but `bottom()` will turn off everything except the geometry for the bottom part of the enclosure.

The sample enclosure is held together with 4 3mm SHCS and requires 4 3mm threaded inserts to placed in the bottom
