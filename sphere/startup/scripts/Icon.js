var icon = [];

function LoadIcons() {
  for (var i = 0; i < list.length; ++i) {
  	var file = OpenFile(list[i].directory + ".dat");
		icon[i] = new Icon(file.read("Icon", "Default.png"),
		                   file.read("Protect", 0));
	}
}

function Icon(file, protect) {
	this.file = file
	this.protect = protect
}