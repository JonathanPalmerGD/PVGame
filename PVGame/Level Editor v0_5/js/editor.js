window.addEventListener("load", init);

var ROWS = 10;
var COLS = 10;

var xmlString = "";

function init()
{
	var container = document.getElementById("container");
	var grid = document.getElementById("grid");

	for (var i = 0; i < ROWS; i++)
	{
		var tr = document.createElement("tr");

		for (var j = 0; j < COLS; j++)
		{
			var td = document.createElement("td");
			td.setAttribute("class", "gridElement");
			td.addEventListener("click",onTileClick);
			tr.appendChild(td);
		}

		grid.appendChild(tr);		
	}
	
	var createXMLButton = document.getElementById("createXMLButton");
	var writeToFileButton = document.getElementById("writeToFileButton");
	
	createXMLButton.addEventListener("click", createXML);
	writeToFileButton.addEventListener("click", outputToFile);
}

function onTileClick()
{	
	var radioButtons = document.getElementsByName("tileType");
	var selected;
	
	for (var i = 0; i < radioButtons.length; i++)
	{
		if (radioButtons[i].checked)
			var selected = radioButtons[i];
	}
	
	if (!selected)
		return;
	
	while (this.childNodes.length > 0)
	{
		this.removeChild(this.firstChild);
	}
	
	switch (selected.value)
	{
		case "wall":
			this.appendChild(document.createTextNode("W"));
			break;
		case "spawn":
			this.appendChild(document.createTextNode("S"));
			break;
		default:
			break;
	}
}

function createXML()
{
	var grid = document.getElementById("grid");
	var walls = new Array();
	var spawns = new Array();
	
	var wallArray;
	var spawnArray;
	
	for (var i = 0; i < ROWS; i++)
	{
		for (var j = 0; j < COLS; j++)
		{
			if (grid.childNodes[i].childNodes[j].firstChild)
			{
				console.log("create element " + grid.childNodes[i].childNodes[j].firstChild.nodeValue + " at row " + i + " col " + j);
				
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue == "W")
				{
					walls.push("\n<wall row=\"" + ((ROWS - i) - 1) + "\" col=\"" + j + "\"/>");
				}
				
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue == "S")
				{
					spawns.push("\n<spawn row=\"" + ((ROWS - i) - 1) + "\" col=\"" + j + "\"/>");
				}
			}
		}
	}
	
	var output = document.getElementById("output");
	
	xmlString = "<level><walls>";
	
	for (var i = 0; i < walls.length; i++)
	{
		xmlString += walls[i];
	}
	
	xmlString += "</walls><spawns>";
	
	for (var i = 0; i < spawns.length; i++)
	{
		xmlString += spawns[i];
	}
	
	xmlString += "</spawns></level>";
	
	while (output.childNodes.length > 0)
	{	
		output.removeChild(output.firstChild);
	}
	
	// Format for readable output
	output.appendChild(document.createTextNode("<level>"));
	output.appendChild(document.createElement("br"));
	output.appendChild(document.createTextNode("<walls>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < walls.length; i++)
	{
		output.appendChild(document.createTextNode(walls[i]));
		output.appendChild(document.createElement("br"));
	}
	
	output.appendChild(document.createTextNode("</walls>"));
	output.appendChild(document.createElement("br"));
	output.appendChild(document.createTextNode("<spawns>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < spawns.length; i++)
	{
		output.appendChild(document.createTextNode(spawns[i]));
		output.appendChild(document.createElement("br"));
	}
	
	output.appendChild(document.createTextNode("</spawns>"));
	output.appendChild(document.createElement("br"));
	output.appendChild(document.createTextNode("</level>"));
}

function outputToFile()
{
	if (window.ActiveXObject)
	{
		var fileName = "level";
		var fileNum = 1;
		
		var fso = new ActiveXObject("Scripting.FileSystemObject");
	
		while (fso.FileExists(fileName + fileNum + ".xml"))
		{
			fileNum++;
		}
	
		var file = fso.CreateTextFile(fileName + fileNum + ".xml", false);
		file.WriteLine(xmlString);
		file.Close();
		
		alert("File created on desktop: " + fileName + fileNum + ".xml");
	}
	
	else
		alert("File creation is currently unavailable on this browser");
	
}