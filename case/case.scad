
BOARDW = 35;
BOARDL = 74;
BOARDH = 1.65;
   
module prism(l, w, h)
{
    polyhedron(
         points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
         faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
    );
}

module pcb(x, y, z)
{

   // main board with mounting holes
   difference()
   {
      color([.3, 0, .7])
      {
         translate([x, y, z])
         cube([BOARDL, BOARDW, BOARDH], true);
      }
      
      // 4 each 4mm diameter holes are 3.5mm from each edge
      color([0, 0, 0])
      {
         translate([x - BOARDL / 2 + 3.5, y - BOARDW / 2 + 3.5, z])
         cylinder(h = BOARDH + 2, d = 4, center = true, $fn=360);
         
         translate([x + BOARDL / 2 - 3.5, y - BOARDW / 2 + 3.5, z])
         cylinder(h = BOARDH + 2, d = 4, center = true, $fn=360);
         
         translate([x + BOARDL / 2 - 3.5, y + BOARDW / 2 - 3.5, z])
         cylinder(h = BOARDH + 2, d = 4, center = true, $fn=360);
         
         translate([x - BOARDL / 2 + 3.5, y + BOARDW / 2 - 3.5, z])
         cylinder(h = BOARDH + 2, d = 4, center = true, $fn=360);
      }
   }
   
   // LEDS
   {
      color([.9, .9, .9])
      translate([x - 2.5, y - 7.5, z + BOARDH + .5])
      cylinder(h = 1.5, d = 8.5, center = true, $fn=360);
      
      color([.3, .3, 1])
      translate([x - 2.5, y - 7.5, z + BOARDH + 3.5])
      cylinder(h = 7, d = 8, center = true, $fn=360);
      
      color([.5, .5, 1])
      translate([x - 2.5, y - 7.5, z + BOARDH + 7])
      sphere(d = 8, $fn=360);
      
      color([.5, .5, 1])
      translate([x - 2.5, y + 8.75, z + BOARDH + .5])
      cylinder(h = 1.5, d = 8.5, center = true, $fn=360);
      
      color([1, .3, .3])
      translate([x - 2.5, y + 8.75, z + BOARDH + 3.5])
      cylinder(h = 7, d = 8, center = true, $fn=360);
      
      color([1, .3, .3])
      translate([x - 2.5, y + 8.75, z + BOARDH + 7])
      sphere(d = 8, $fn=360);
   }
   
   // ESP32-S3
   color([0, .3, 0])
   translate([x - BOARDL / 2, y - BOARDW / 4, z + BOARDH + 1])
   cube([22, 18, 2], false);
   
   // ESP32-S3 USB
   color([.765, .765, .765])
   {
      translate([x - BOARDL / 2 - 2, y - BOARDW / 4 + 4.5, z + BOARDH + 4.5])
      rotate([0, 90, 0])
      cylinder(h=7 , d=3, $fn=360);
      
      //color([.765, .765, .765])
      translate([x - BOARDL / 2 - 2, y - BOARDW / 4 + 13.5, z + BOARDH + 4.5])
      rotate([0, 90, 0])
      cylinder(h=7 , d=3, $fn=360);
      
      //color([.765, .765, .765])
      translate([x - BOARDL / 2 - 2, y - BOARDW / 4 + 4.5, z + BOARDH + 3])
      cube([7, 9, 3], false);
   }
   
   // ESP32-S3 mounting headers
   color([0,0,0])
   {
      translate([x - BOARDL / 2 + 2, y - BOARDW / 2 + 9, z + BOARDH / 2])
      cube([18, 2, 2], false);
      
      translate([x - BOARDL / 2 + 2, y + BOARDW / 2 - 11, z + BOARDH / 2])
      cube([18, 2, 2], false);
   }
}

module foofraw(width, length, height)
{
   color([0, 0, 0])
   {
      translate([-width / 2, length / 2, 0])
      cube([width, 5, height], false);
      
      translate ([-20, length / 2 + 5, height + 4])
      rotate([0, 90, 270])
      prism(4, 5, 5);
      
      translate([-15, length / 2, height])
      cube([width / 2 + 15, 5, 4], false);
      
      
      translate([-width / 2, -length / 2 - 5, 0])
      cube([width, 5, height], false);
      
      translate ([-20, -length / 2 - 5, height ])
      rotate([0, 270, 270])
      prism(4, 5, 5);
      
      translate([-15, -length / 2 -5, height])
      cube([width / 2 + 15, 5, 4], false);
   }   
}


module bottom(width, length, height)
{
   WALLTHICK = 20;
   
   difference()
   {
      color([0, 0, 0])
      {
         translate([-width / 2, -length / 2, 0])
         cube([width, length, height], false);
      }
      
      // main well
      color([0, 0, 1])
      {
         translate([-(width - WALLTHICK) / 2, -(length - WALLTHICK) / 2, 3])
         cube([width - WALLTHICK, length - WALLTHICK, height - height + 20], false);
      }
      
      // board well 1
      color ([.3, .3, 1])
      {
          translate([-width / 2 + 2, -length / 2 + 2, 10])
          cube([79.5, 38, 15], false);
      }
      
      // board well 2
      color ([.3, .3, 1])
      {
          translate([-width / 2 + 10, -length / 2 + 2, 3])
          cube([58, 38, 15], false);
      }
      
      // board well 3
      color([.3, .3, 1])
      {
          translate([-width / 2 + 2, -length / 2 + 10, 3])
          cube([58, 22, 15], false);
      }
      
      // usb well
      color ([.1, .4, 1])
      {
          translate([-width / 2 - 2, -length / 2 + 13, 13])
          cube ([8, 16, 15], false);
      }
      

      // diamonds 1
      for(yy = [-7:10:10])
      {
         for (xx = [-25:10:30])
         {
            color([0, 1, 1])
            translate([xx+.85 - 3, yy-1.5, -1])
            rotate([0, 0, 45])
            cube([5, 5, 4.01], false);
         }
      }
      
      // diamonds 2
      for (yy = [-13:10:12])
      {
         for (xx = [-30:10:30])
         {
            color([0, 1, 1])
            translate([xx + .9 -3, yy - .4, -1])
            rotate([0, 0, 45])
            cube([5, 5, 5], false);
         }
      }

      // holes for threaded inserts
      color([1, .7568, .4314])
      {
         translate([-5 - BOARDL / 2 + 3.5, 0 - BOARDW / 2 + 3.5, 10])
         cylinder(h = 18, d = 4, center = true, $fn=360);
         
         translate([-5 + BOARDL / 2 - 3.5, 0 - BOARDW / 2 + 3.5, 10])
         cylinder(h = 18, d = 4, center = true, $fn=360);
         
         translate([-5 + BOARDL / 2 - 3.5, 0 + BOARDW / 2 - 3.5, 10])
         cylinder(h = 18, d = 4, center = true, $fn=360);
         
         translate([-5 - BOARDL / 2 + 3.5, 0 + BOARDW / 2 - 3.5, 10])
         cylinder(h = 18, d = 4, center = true, $fn=360);
      }
      
      // antenna openings
      color([0, 0, 1])
      {
          translate([width / 2, 0, height / 2 + 1])
          cube([30, 13, 13], true);
          translate([width / 2, 0, height / 2 + 5])
          cube([30, 13, 13], true);
      }
      color([.5, 0, .5])
      {
          translate([width / 2 - 7, -8, height / 2 - 8])
          cube([6, 17, 20], false);
          
          translate([width / 2 - 11, 5, height / 2 - 1])
          cube([4, 10, 6], false);
        }
    }
    
    difference()
    {
    foofraw(width, length, height);
    
    color([.1, .1, .1])
    {
       for (xx = [-10:10:30])
       {
         translate([xx, length / 2 + 5, height / 2])
         rotate([0, 0, 45])
         cube([5, 5, height + 10], true);
       
         translate([xx, -length / 2 - 5, height / 2])
         rotate([0, 0, 45])
         cube([5, 5, height + 10], true);
       }
       
      translate([width / 2, length / 2 + 5, height / 2])
      rotate([0, 0, 45])
      cube([5, 5, height + 10], true);
    
      translate([width / 2, -length / 2 - 5, height / 2])
      rotate([0, 0, 45])
      cube([5, 5, height + 10], true);
    }
  }
}


module top(width, length, height, top)
{
    standoffHeight = 10 - (BOARDH / 2);
    
    difference()
    {
        color([1, .8, 0])
        {
            // main top body
            translate([0, 0, top + height / 2])
            cube([width, length, height], true);
            
            // antenna holder
            translate([width / 2 - 5, -6.5, top - 4])
            cube([5, 13, 4], false);
             
            // board holders/standoffs
            translate([-5 - BOARDL / 2 + 3.5, 0 - BOARDW / 2 + 3.5, 10 + (BOARDH) + standoffHeight / 2])
            cube([7, 7, standoffHeight], true);
            
            translate([-5 + BOARDL / 2 - 3.5, 0 - BOARDW / 2 + 3.5, 10 + (BOARDH) + standoffHeight / 2])
            cylinder(h=standoffHeight, d=7, center=true, $fn=360);
            
            translate([-5 + BOARDL / 2 - 3.5, 0 + BOARDW / 2 - 3.5, 10 + (BOARDH) + standoffHeight / 2])
            cylinder(h=standoffHeight, d=7, center=true, $fn=360);
            
            translate([-5 - BOARDL / 2 + 3.5, 0 + BOARDW / 2 - 3.5, 10 + (BOARDH) + standoffHeight / 2])
            cube([7, 7, standoffHeight], true);
        }
        
        // LED
        color ([1, .8, 0])
        {
          translate([-5 - 2.5, -7.5, top])
          cylinder(h = 10, d = 11, center = true, $fn=360);
          
          translate([-5 - 2.5, 8.75, top])
          cylinder(h = 10, d = 11, center = true, $fn=360);
            
          translate([-5 - 2.5, 1.25, top])
          cube([11, 16, 10], true);
        }
        
        // 3mm relief
        color([.1, .1, .1])
        {
           translate([-5 - BOARDL / 2 + 3.5, 0 - BOARDW / 2 + 3.5, 10 + (BOARDH / 2) + standoffHeight / 2])
           cylinder(h = 22, d = 3.7, center = true, $fn=360);
           
           translate([-5 + BOARDL / 2 - 3.5, 0 - BOARDW / 2 + 3.5, 10 + (BOARDH / 2) + standoffHeight / 2])
           cylinder(h = 22, d = 3.7, center = true, $fn=360);
            
           translate([-5 + BOARDL / 2 - 3.5, 0 + BOARDW / 2 - 3.5, 10 + (BOARDH / 2) + standoffHeight / 2])
           cylinder(h = 22, d = 3.7, center = true, $fn=360);
           
           translate([-5 - BOARDL / 2 + 3.5, 0 + BOARDW / 2 - 3.5, 10 + (BOARDH / 2) + standoffHeight / 2])
           cylinder(h = 22, d = 3.7, center = true, $fn=360);
        }   
        
        // 3mm counterbore
        color([.2, .2, .2])
        {
           translate([-5 - BOARDL / 2 + 3.5, 0 - BOARDW / 2 + 3.5, top + height / 2 + 1.01])
           cylinder(h = 3, d = 6.5, center = true, $fn=360);
           
           translate([-5 + BOARDL / 2 - 3.5, 0 - BOARDW / 2 + 3.5, top + height / 2 + 1.01])
           cylinder(h = 3, d = 6.5, center = true, $fn=360);
            
           translate([-5 + BOARDL / 2 - 3.5, 0 + BOARDW / 2 - 3.5, top + height / 2 + 1.01])
           cylinder(h = 3, d = 6.5, center = true, $fn=360);
           
           translate([-5 - BOARDL / 2 + 3.5, 0 + BOARDW / 2 - 3.5, top + height / 2 + 1.01])
           cylinder(h = 3, d = 6.5, center = true, $fn=360);
        }   
    }
    
    color([1, .8, 0])
    {
       translate([-width / 2, length / 2, top])
       cube([width / 2 - 20, 5, 4], false);
      
       translate ([-15, length / 2, top + 4])
       rotate([0, 90, 90])
       prism(4, 5, 5);
       
       
       translate([-width / 2, -length / 2 - 5, top])
       cube([width / 2 - 20, 5, 4], false);
             
       translate ([-15, -length / 2, top])
       rotate([0, 270, 90])
       prism(4, 5, 5);
    }
}

module antenna()
{
    color([1,.922,.804])
    translate ([38.5, -7, 2])
    cube([5, 15, 15], false);
}


module lanyardRing(x, y, z)
{
   translate([x, y, z])
   {
      difference()
      {
      color([0, 0, 0])
         {
            translate([0, -4, 5])
            rotate([0, 90, 0])
            prism(10, 4, 8);
            
            translate([0, 4, -5])
            rotate([0, 270, 180])
            prism(10, 4, 8);
            
            translate([7, 0, 0])
            rotate([90, 0, 0])
            cylinder(h=2, d=10, center=true, $fn=360);
         }
         
         color([0, 0, 0])
         translate([7, 0, 0])
         rotate([90, 0, 0])
         cylinder(h=8, d=6.5, center=true, $fn=360);
      }
   }
}

module posts()
{
  color([1, .7568, .4314])
  {
     translate([-5 - BOARDL / 2 + 3.5, 0 - BOARDW / 2 + 3.5, 15])
     cylinder(h = 22, d = 3, center = true, $fn=360);
     
     translate([-5 + BOARDL / 2 - 3.5, 0 - BOARDW / 2 + 3.5, 15])
     cylinder(h = 22, d = 3, center = true, $fn=360);
     
     translate([-5 + BOARDL / 2 - 3.5, 0 + BOARDW / 2 - 3.5, 15])
     cylinder(h = 22, d = 3, center = true, $fn=360);
     
     translate([-5 - BOARDL / 2 + 3.5, 0 + BOARDW / 2 - 3.5, 15])
     cylinder(h = 22, d = 3, center = true, $fn=360);
  }    
}






module main()
{
    // the whole shebang
    //bottom(90, 42, 20);
    top(90, 42, 4, 20);
    //pcb(-5, 0, 10);
    //antenna();
    //lanyardRing(42, 18, 12);
    //posts();
}


difference()
{
   main();
   
   color([0, 0, 0])
   {
      translate([-45, 26, 10])
      rotate([0, 0, 45])
      cube([8, 8, 30], true);
    
      translate([-45, -26, 10])
      rotate([0, 0, 45])
      cube([8, 8, 30], true);
   }
}


