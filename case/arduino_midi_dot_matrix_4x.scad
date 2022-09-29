// arduino pro micro and four 8x8 dot-matrix led display

//micro usb
usb_height=2.7;
usb_width=9.0;
arduino_length=33.5 + 1.6*2 /* mount */;
arduino_offset_x=2.0; // required because of dot-matrix display connector is in the way

pcb_width=32.2;
pcb_length=130 + arduino_length;
pcb_height=1.6;

inner_height=2.5; // height on top of pcb
inner_height_below=10; // height below pcb
bottom=1.0; // bottom layer thickness

sides=1.0; //mm thickness

enable_pcb_slide_in=0;
enable_pcb_mount=1; // extra pcb mounting rail
enable_cover_rim=1;
enable_led_hole=1;

led_diameter=5.1;

side_diff=0.7; // pcb slide-in in mm

extra_length=1;

width=pcb_width + 2*sides - (enable_pcb_slide_in==1 ? 2*side_diff : 0);
inner_width = pcb_width - (enable_pcb_slide_in==1 ? 2*side_diff : 0);
length=pcb_length + 2*sides - side_diff + extra_length;//extra
inner_length= pcb_length + 1*sides - side_diff + extra_length;
height=bottom+pcb_height+inner_height+sides+inner_height_below;

part=4; // 1=main case, 2=only arduino holder, 3=cover, 4=back cover
e=0.01;
enable_air_flow=0;

module arduino_mount(board="micro", width=18, length=33.5, pcb=1.8, preview=1, height=5, holder_width=3, enable_back_slide=1, tolerance=0.2, center=true) {
  inward=2;
  thick=holder_width;
  top=3;
  offset_x=center==true ? width/2 : 0;
  translate([offset_x,thick/2,0]){
    difference() {
      union() {
        //front holder:
        translate([inward, -thick/2, 0]) cube([thick, thick/2-tolerance, height+top]);
        translate([width-inward-thick, -thick/2, 0]) cube([thick, thick/2-tolerance, height+top]);
        translate([inward, -thick/2, height+pcb]) cube([thick, thick, top/3]);
        translate([width-inward-thick, -thick/2, height+pcb]) cube([thick, thick, top/3]);
        //middle
        translate([width/2-thick, -thick/2, 0]) cube([thick*2, thick, height+pcb]);
        
        //left side:
        translate([-thick/2, +thick, 0]) cube([thick/2, thick, height+pcb-e]);
        translate([-thick/2, length-thick-thick, 0]) cube([thick/2, thick, height+pcb-e]);
        //right side:
        translate([width, +thick, 0]) cube([thick/2, thick, height+pcb-e]);
        translate([width, length-thick-thick, 0]) cube([thick/2, thick, height+pcb-e]);
        
        //backside slide in (where the usb connector is):
        if(enable_back_slide==1) {
          translate([-thick/2, length-thick/2, 0]) cube([thick, thick, height+top]);
          translate([width-thick/2, length-thick/2, 0]) cube([thick, thick, height+top]);
        }
      }
    
      color([0.5,1,0.5]) translate([-tolerance,-tolerance,height]) cube([width+2*tolerance, length+2*tolerance, pcb+tolerance]);
      top_extra=0.25;
      translate([inward-e, -thick/2, height+top+top_extra]) rotate([-28,0,0]) cube([thick+2*e, thick*3, height+top]);
      translate([width-inward-thick-e, -thick/2, height+top+top_extra]) rotate([-28,0,0]) cube([thick+3*e, thick*3, height+top]);
    }
    if(preview==1)
      color([0.5,1,0.5]) translate([0,0,height]) cube([width, length, pcb]);
  }
}


module print_stand_mount() {
  // stand:
  stand_at=15;
  stand_width=3;
  translate([-e,stand_at, height-bottom/2]) color([1,0,0]) cube([width+2*e, stand_width, bottom]); 
  translate([-e,length - stand_at, height-bottom/2]) color([1,0,0]) cube([width+2*e, stand_width, bottom]); 
}


if(part==4) { // back Cover
  cover_width=width-2*sides;
  cover_height=height-sides;
  top=2.4;
  cube([cover_width, cover_height, sides]);
  translate([0,0,0]) cube([cover_width, sides, top]);
}


if(part==3) { // Cover
  inward=sides;
  cover_length=36.5-0.25;
  cover_width=width-2*sides;
  clips=4;
  clip_height=sides*2;
  clip_length=2;
  clip_thick=3;
  clip_dist=(cover_length - 2*inward - clip_length*2)/(clips-1);
  union() {
    cube([cover_width, cover_length, sides]);
    
    for(i=[0 : 1 : clips-1]) color([0,1,0]) {
      translate([0,i*clip_dist,0]) difference() {
        union() {
          translate([sides, inward, sides-e]) cube([clip_thick, clip_length, clip_height]);
          
          translate([cover_width-sides-clip_thick, inward, sides-e]) cube([clip_thick, clip_length, clip_height]);
        }
      
        translate([sides, inward-e, 2*sides]) rotate([-90,0,0]) cylinder(r=sides, h=clip_length+2*e, $fn=32);
        translate([cover_width-sides, inward-e, 2*sides]) rotate([-90,0,0]) cylinder(r=sides, h=clip_length+2*e, $fn=32);
      }
  }
  }
}

if(part==2) {
  arduino_mount();
}

//rotate([0,180,0])
//rotate([0,0,180])
translate([0,-length/2,0])
if(part==1) {
  
  translate([width+arduino_offset_x,length-arduino_length,height]) rotate([0,180,0]) arduino_mount(preview=0);
  
  if(enable_pcb_mount==1) {
    into=sides+side_diff;
    mount_width=2.5;
    mount_height=inner_height+e;
    mounts=5;
    tolerance=0.01;
    spacing = (inner_length - arduino_length - 10*into) / (mounts-1);
    for(i=[0:1:mounts-1]) {
      translate([sides, into+i*spacing, bottom + inner_height_below+pcb_height]) cube([mount_width, mount_width, mount_height]);
      translate([width-sides-mount_width, into+i*spacing, bottom + inner_height_below+pcb_height]) cube([mount_width, mount_width, mount_height]);
    }
    
    //top for slidein
    translate([0,0,-mount_height-pcb_height-tolerance]) for(i=[0:1:mounts-1]) {
      translate([sides, into+i*spacing, bottom + inner_height_below+pcb_height]) difference() {
        cube([mount_width, mount_width, mount_height]);
        
        translate([-sides,-e,mount_height]) rotate([0,20,0]) cube([mount_width*2, mount_width+2*e, mount_height]);
      }
      translate([width-sides-mount_width, into+i*spacing, bottom + inner_height_below+pcb_height]) { 
        rotate([0,0,180]) translate([-mount_width,-mount_width,0]) difference() {
          cube([mount_width, mount_width, mount_height]);
          translate([0,-e, mount_height]) rotate([0,20,0]) cube([mount_width*2, mount_width+2*e, mount_height]);
        } 

      }
    }
  }
  
  // rim for holding arduino cover
  if(enable_cover_rim==1) color([0,1,0]) {
    translate([sides,length-2*sides, sides]) cube([sides,sides,sides]);
    translate([sides,length-arduino_length,sides*2])  {
      translate([0,0,-sides]) cube([sides,sides,sides]);
      rotate([180+90,0,0])  cylinder(r=sides, h=arduino_length, $fn=32);
    }
    translate([width-2*sides,length-2*sides, sides]) cube([sides,sides,sides]);
    translate([width-sides,length-arduino_length,sides*2]) {
      translate([-sides,0,-sides]) cube([sides,sides,sides]);
      rotate([180+90,0,0]) cylinder(r=sides, h=arduino_length, $fn=32);
    }
  }
  
  if(part==1)difference() {

    union() {
      cube([width, length, height]);
      
      
    }
    
    // main inner hull
    translate([sides, -e, -e]) cube([inner_width, inner_length+e, sides + pcb_height + inner_height + inner_height_below]);
    
    // pcb slide ins:
    if(enable_pcb_slide_in==1) {
      translate([sides-side_diff, -e, bottom + inner_height_below]) cube([pcb_width, inner_length+side_diff+2*e, pcb_height]);
    }
    
    
    // usb connector:
    translate([width/2-sides/2-usb_width/2+arduino_offset_x, length-sides-side_diff-e, bottom + inner_height_below]) cube([usb_width, 2*sides, usb_height+pcb_height]);
    
    /*
    translate([width-sides-17, length-1-e, bottom+pcb_height+3*bottom]) {
    rotate([-90,0,0]) color([1,1,0.5])
      linear_extrude(2)
              text("OUT", size=8, halign="center",valign="top",font="Freesans:style=Bold", spacing=1);
    }
    */
    
    print_stand_mount();
    
    // front door slide in
    translate([sides-(enable_pcb_slide_in==1?side_diff:side_diff/2), sides, -e]) color([0,1,0])
      cube([width-2*sides+ (enable_pcb_slide_in==1?2*side_diff:side_diff), side_diff, height-sides+side_diff]);
    
    // air flow
    if(enable_air_flow==1) {
      translate([-e, sides+extra_length+10, bottom+pcb_height+2]) {
        wi=2;
        cnt=10;
        for(i=[0:1:cnt+1]) {
          translate([0,i*3*wi, 0]) cube([sides+e, wi, height/2]);
        }
      }
    }
    
    if(enable_led_hole==1) {
      translate([-e, length - arduino_length/2 - sides, height/2])
      color([1,0.5,1]) rotate([0,90,0])
      cylinder(r=led_diameter/2, h=sides*2, $fn=32);
    }
  }//difference
  //print_stand();
  
  
  
}