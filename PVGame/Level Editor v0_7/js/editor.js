window.addEventListener("load", init);

var ROWS = 25;
var COLS = 25;
var MAX_ROOMS = 50;
var MAX_TRANSLATE_X = 50;
var MAX_TRANSLATE_Y = 10;
var MAX_TRANSLATE_Z = 50;
var MAX_WALL_HEIGHT = 10;

var isMouseDown = false;
var xmlString = "";

function init()
{
	window.addEventListener("mousedown", function(){isMouseDown = true;});
	window.addEventListener("mouseup", function(){isMouseDown = false;});
	
	var gridToggle = document.getElementById("gridToggle");
	gridToggle.addEventListener("click", toggleGrid);
	
	var container = document.getElementById("container");
	var grid = document.getElementById("grid");

	var wallHeight = document.getElementById("wallHeight");
	
	for (var i = 0; i < MAX_WALL_HEIGHT; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		wallHeight.appendChild(option);
	}
	
	var exitNum = document.getElementById("exitNum");
	
	var ucTranslateX = document.getElementById("ucTranslateX");
	var ucTranslateY = document.getElementById("ucTranslateY");
	var ucTranslateZ = document.getElementById("ucTranslateZ");
	
	for (var i = 0; i < ROWS; i++)
	{
		var tr = document.createElement("tr");

		for (var j = 0; j < COLS; j++)
		{
			var td = document.createElement("td");
			td.setAttribute("class", "gridElement");
			td.addEventListener("mousedown",onTileClick);
			td.addEventListener("mouseover",onTileClick);
			tr.appendChild(td);
		}

		grid.appendChild(tr);		
	}
	
	for (var i = 0; i < MAX_ROOMS; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		exitNum.appendChild(option);
	}
	
	for (var i = -MAX_TRANSLATE_X; i < MAX_TRANSLATE_X; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		ucTranslateX.appendChild(option);
	}
	
	for (var i = -MAX_TRANSLATE_Y; i < MAX_TRANSLATE_Y; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		ucTranslateY.appendChild(option);
	}
	
	for (var i = -MAX_TRANSLATE_Z; i < MAX_TRANSLATE_Z; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		ucTranslateZ.appendChild(option);
	}
	
	var writeToFileButton = document.getElementById("writeToFileButton");
	writeToFileButton.addEventListener("click", outputToFile);
}

function toggleGrid()
{
	var grid = document.getElementById("grid");
	
	if (grid.getAttribute("style") == "display: block")
		grid.setAttribute("style", "display: none");
	else
		grid.setAttribute("style", "display: block");
}

function onTileClick(evt)
{
	if (evt.type == "mousedown")
		isMouseDown = true;
	
	if (!isMouseDown)
		return;
	
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
	
	var div = document.createElement("div");
	
	switch (selected.value)
	{
		case "wall":
			this.setAttribute("style", "background-color: #FF0000; color: #FFFFFF;");
			this.appendChild(document.createTextNode("W|" + document.getElementById("wallHeight").value));
			break;
		case "spawn":
			this.setAttribute("style", "background-color: #0000FF; color: #FFFFFF;");
			this.appendChild(document.createTextNode("Spawn " + document.getElementById("spawnDirection").value));
			break;
		case "exit":
			this.setAttribute("style", "background-color: #FFFF00; color: #000000;");
			this.appendChild(document.createTextNode("level" + document.getElementById("exitNum").value + ".xml"));
			break;
		case "cube":
			this.setAttribute("style", "background-color: #862125; color: #FFFFFF;");
			this.appendChild(document.createTextNode("c|" + document.getElementById("ucTranslateX").value + "|" + document.getElementById("ucTranslateY").value + "|" +
					 document.getElementById("ucTranslateZ").value));
			break;
		case "crest":
			this.setAttribute("style", "background-color: #42CD76; color: #000000;");
			this.appendChild(document.createTextNode("Crest:" + document.getElementById("crestName").value + ":" + document.getElementById("unlockCube").value));
			break;
		default:
			this.setAttribute("style", "background-color: #FFFFFF; color: #000000;");
			break;
	}
	
	createXML();
}

function createXML()
{
	var grid = document.getElementById("grid");
	var walls = new Array();
	var spawns = new Array();
	var exits = new Array();
	var cubes = new Array();
	var crests = new Array();
	
	for (var i = 0; i < ROWS; i++)
	{
		for (var j = 0; j < COLS; j++)
		{
			if (grid.childNodes[i].childNodes[j].firstChild)
			{	
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue[0] == "W")
					walls.push({row: ((ROWS - i) - 1), col: j, yLength: grid.childNodes[i].childNodes[j].firstChild.nodeValue.substr(2)});
				
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue[0] == "S")
					spawns.push({row: ((ROWS - i) - 1), col: j, dir: grid.childNodes[i].childNodes[j].firstChild.nodeValue.substr(6)});
				
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue[0] == "l")
					exits.push({row: ((ROWS - i) - 1), col: j, file: grid.childNodes[i].childNodes[j].firstChild.nodeValue});
					
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue[0] == "c")
				{
					var strArray = grid.childNodes[i].childNodes[j].firstChild.nodeValue.split("|");
					
					cubes.push({row: ((ROWS - i) - 1), col: j, translateX: strArray[1], translateY: strArray[2], translateZ: strArray[3]});	
				}
					
				if (grid.childNodes[i].childNodes[j].firstChild.nodeValue[0] == "C")
				{
					var strArray = grid.childNodes[i].childNodes[j].firstChild.nodeValue.split(":");
					var targetCube;
					
					if (strArray[1] == 3)
						targetCube = strArray[2];
					else
						targetCube = "";
					
					crests.push({row: ((ROWS - i) - 1), col: j, effect: strArray[1], target: targetCube});
				}
			}
		}
	}
		
	walls = combineElements(walls);
	spawns = combineElements(spawns);
	exits = combineElements(exits);
	cubes = combineElements(cubes);
	crests = combineElements(crests);
	
	listCubes(cubes);
	writeXML(walls, spawns, exits, cubes, crests);
}

function combineElements (elementArray)
{
	if (elementArray.length == 0)
		return elementArray;
	
	var wallRowCol = new Array(ROWS);
	
	for (var i = 0; i < wallRowCol.length; i++)
	{
		wallRowCol[i] = new Array();
	}
	
	for (var i = 0; i < elementArray.length; i++)
	{		
		var tempWall = {row: 0.0, col: -1.0, xLength: 1.0, yLength: 1.0, zLength: 1.0, centerX: 0.0, centerY: 0.0, centerZ: 0.0, translateX: 0.0, translateY: 0.0, translateZ: 0.0,
				dir: undefined, file: undefined, effect: undefined, target: undefined};

		tempWall.row = elementArray[i].row;
		tempWall.col = elementArray[i].col;
		tempWall.yLength = elementArray[i].yLength;
		tempWall.dir = elementArray[i].dir;
		tempWall.file = elementArray[i].file;
		tempWall.effect = elementArray[i].effect;
		tempWall.target = elementArray[i].target;
		tempWall.centerX = tempWall.col + tempWall.xLength / 2;
		tempWall.centerZ = tempWall.row + tempWall.zLength / 2;
		tempWall.translateX = elementArray[i].translateX;
		tempWall.translateY = elementArray[i].translateY;
		tempWall.translateZ = elementArray[i].translateZ;
		
		wallRowCol[elementArray[i].row].push(tempWall);
	}
	
	if (elementArray[0].effect != undefined)
		return wallRowCol;
	
	// Combine rows
	for (var i = 0; i < wallRowCol.length; i++)
	{	
		for (var j = 0; j < wallRowCol[i].length - 1; j++)
		{	
			if ((wallRowCol[i].length > 0) && ((wallRowCol[i][j].col + wallRowCol[i][j].xLength) == wallRowCol[i][j + 1].col))
			{
				
				if (((wallRowCol[i][j].dir == undefined) || (((wallRowCol[i][j].dir != undefined) && (wallRowCol[i][j].dir == wallRowCol[i][j + 1].dir)))) &&
				   ((wallRowCol[i][j].file == undefined) || (((wallRowCol[i][j].file != undefined) && (wallRowCol[i][j].file == wallRowCol[i][j + 1].file)))) &&
				   (wallRowCol[i][j].yLength == wallRowCol[i][j + 1].yLength) && ((wallRowCol[i][j].translateX == wallRowCol[i][j + 1].translateX) &&
				   (wallRowCol[i][j].translateY == wallRowCol[i][j + 1].translateY) && (wallRowCol[i][j].translateZ == wallRowCol[i][j + 1].translateZ)))
				{	
					wallRowCol[i].splice(j + 1, 1);

					wallRowCol[i][j].xLength++;
					wallRowCol[i][j].centerX = wallRowCol[i][j].col + wallRowCol[i][j].xLength / 2;
					wallRowCol[i][j].centerZ = wallRowCol[i][j].row + wallRowCol[i][j].zLength / 2;
	
					j--;
				}
			}

			else
			{
				if (wallRowCol[i].length == 0 && i < wallRowCol.length - 1)
				{
					i++;
					j = 0;
				}
			}
		}
	}
	
	// Combine columns: INCREDIBLY ugly, but necessary because the 2d array is sorted by row, not column.
	for (var i = 0; i < wallRowCol.length - 1; i++)
	{
		for (var j = 0; j < wallRowCol.length; j++)
		{
			for (var k = 0; k < wallRowCol[i].length; k++)
			{
				for (var l = 0; l < wallRowCol[i].length; l++)
				{			
					if ((wallRowCol[i + j] != undefined) && (wallRowCol[i + j][k] != undefined) &&
					    ((wallRowCol[i][l].row + wallRowCol[i][l].zLength) == wallRowCol[i + j][k].row) && 
					    (wallRowCol[i][l].col == wallRowCol[i + j][k].col) && (wallRowCol[i][l].xLength == 1) && (wallRowCol[i + j][k].xLength == 1))
					{
						
						if (((wallRowCol[i][l].dir == undefined) || (((wallRowCol[i][l].dir != undefined) && (wallRowCol[i][l].dir == wallRowCol[i + j][k].dir)))) &&
						   ((wallRowCol[i][l].file == undefined) || (((wallRowCol[i][l].file != undefined) && (wallRowCol[i][l].file == wallRowCol[i + j][k].file)))) &&
						   (wallRowCol[i][l].yLength == wallRowCol[i + j][k].yLength) && ((wallRowCol[i][l].translateX == wallRowCol[i + j][k].translateX) &&
						   (wallRowCol[i][l].translateY == wallRowCol[i + j][k].translateY) && (wallRowCol[i][l].translateZ == wallRowCol[i + j][k].translateZ)))
						{
							wallRowCol[i + j].splice(k, 1);
	
							if (wallRowCol[i + j].length == 0)
								wallRowCol.splice(i + j, 1);
	
							wallRowCol[i][l].zLength++;
							wallRowCol[i][l].centerX = wallRowCol[i][l].col + wallRowCol[i][l].xLength / 2;
							wallRowCol[i][l].centerZ = wallRowCol[i][l].row + wallRowCol[i][l].zLength / 2;
						
							i = 0;
							j = 0;
							k = 0;
							l = -1;
						}
					}
				}	
			}
		}
	}
	
	return wallRowCol;
}

function listCubes(cubes)
{
	var unlockCube = document.getElementById("unlockCube");
	unlockCube.innerHTML = "";
	
	for (var i = 0; i < cubes.length; i++)
	{
		for (var j = 0; j < cubes[i].length; j++)
		{
			var option = document.createElement("option");
		
			option.value = cubes[i][j].row + "|" + cubes[i][j].col;
			option.appendChild(document.createTextNode(option.value));
			
			unlockCube.appendChild(option);
		}	
	}
}

function writeXML(walls, spawns, exits, cubes, crests)
{
	var output = document.getElementById("output");
	
	xmlString = "<level><walls>";
	
	for (var i = 0; i < walls.length; i++)
	{
		for (var j = 0; j < walls[i].length; j++)
		{
			xmlString += "<wall row=\"" + walls[i][j].row + "\" col=\"" + walls[i][j].col + "\" xLength=\"" + walls[i][j].xLength +  "\" yLength=\"" + walls[i][j].yLength +
				     "\" zLength=\"" + walls[i][j].zLength + "\" centerX=\"" + walls[i][j].centerX + "\" centerY=\"" + walls[i][j].centerY +
				     "\" centerZ=\"" + walls[i][j].centerZ + "\"/>";
		}
	}
	
	xmlString += "</walls><spawns>";
	
	for (var i = 0; i < spawns.length; i++)
	{
		for (var j = 0; j < spawns[i].length; j++)
		{
			xmlString += "<spawn row=\"" + spawns[i][j].row + "\" col=\"" + spawns[i][j].col + "\" xLength=\"" + spawns[i][j].xLength +
				     "\" zLength=\"" + spawns[i][j].zLength + "\" centerX=\"" + spawns[i][j].centerX + "\" centerY=\"" + spawns[i][j].centerY +
				     "\" centerZ=\"" + spawns[i][j].centerZ + "\" dir=\"" + spawns[i][j].dir + "\"/>";
		}
	}
	
	xmlString += "</spawns><exits>";
	
	for (var i = 0; i < exits.length; i++)
	{
		for (var j = 0; j < exits[i].length; j++)
		{
			xmlString += "<exit row=\"" + exits[i][j].row + "\" col=\"" + exits[i][j].col + "\" xLength=\"" + exits[i][j].xLength +
				     "\" zLength=\"" + exits[i][j].zLength + "\" centerX=\"" + exits[i][j].centerX + "\" centerY=\"" + exits[i][j].centerY +
				     "\" centerZ=\"" + exits[i][j].centerZ + "\" file=\"" + exits[i][j].file + "\"/>";
		}
	}
	
	
	xmlString += "</exits><cubes>";
	
	for (var i = 0; i < cubes.length; i++)
	{	
		for (var j = 0; j < cubes[i].length; j++)
		{
			xmlString += "<cube row=\"" + cubes[i][j].row + "\" col=\"" + cubes[i][j].col + "\" xLength=\"" + cubes[i][j].xLength +
				     "\" zLength=\"" + cubes[i][j].zLength + "\" centerX=\"" + cubes[i][j].centerX + "\" centerY=\"" + cubes[i][j].centerY +
				     "\" centerZ=\"" + cubes[i][j].centerZ + "\" translateX=\"" + cubes[i][j].translateX + "\" translateY=\"" + cubes[i][j].translateY +
				     "\" translateZ=\"" + cubes[i][j].translateZ + "\"/>";
		}
	}
	
	xmlString += "</cubes><crests>";
	
	for (var i = 0; i < crests.length; i++)
	{	
		for (var j = 0; j < crests[i].length; j++)
		{
			xmlString += "<crest row=\"" + crests[i][j].row + "\" col=\"" + crests[i][j].col + "\" xLength=\"" + crests[i][j].xLength +
				     "\" zLength=\"" + crests[i][j].zLength + "\" centerX=\"" + crests[i][j].centerX + "\" centerY=\"" + crests[i][j].centerY +
				     "\" centerZ=\"" + crests[i][j].centerZ + "\" effect=\"" + crests[i][j].effect + "\" target=\"" + crests[i][j].target + "\"/>";
		}
	}
	
	xmlString += "</crests><level>";	
	
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
		for (var j = 0; j < walls[i].length; j++)
		{
			output.appendChild(document.createTextNode("<wall row=\"" + walls[i][j].row + "\" col=\"" + walls[i][j].col + "\" xLength=\"" + walls[i][j].xLength +  "\" yLength=\"" + walls[i][j].yLength +
								   "\" zLength=\"" + walls[i][j].zLength + "\" centerX=\"" + walls[i][j].centerX + "\" centerY=\"" + walls[i][j].centerY +
								   "\" centerZ=\"" + walls[i][j].centerZ + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</walls>"));
	output.appendChild(document.createElement("br"));
	output.appendChild(document.createTextNode("<spawns>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < spawns.length; i++)
	{
		for (var j = 0; j < spawns[i].length; j++)
		{
			output.appendChild(document.createTextNode("<spawn row=\"" + spawns[i][j].row + "\" col=\"" + spawns[i][j].col + "\" xLength=\"" + spawns[i][j].xLength +
								   "\" zLength=\"" + spawns[i][j].zLength + "\" centerX=\"" + spawns[i][j].centerX + "\" centerY=\"" + spawns[i][j].centerY +
								   "\" centerZ=\"" + spawns[i][j].centerZ + "\" dir=\"" + spawns[i][j].dir + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</spawns>"));
	output.appendChild(document.createElement("br"));
	
	output.appendChild(document.createTextNode("<exits>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < exits.length; i++)
	{
		for (var j = 0; j < exits[i].length; j++)
		{
			output.appendChild(document.createTextNode("<exit row=\"" + exits[i][j].row + "\" col=\"" + exits[i][j].col + "\" xLength=\"" + exits[i][j].xLength +
								   "\" zLength=\"" + exits[i][j].zLength + "\" centerX=\"" + exits[i][j].centerX + "\" centerY=\"" + exits[i][j].centerY +
								   "\" centerZ=\"" + exits[i][j].centerZ + "\" file=\"" + exits[i][j].file + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</exits>"));
	output.appendChild(document.createElement("br"));
	
	output.appendChild(document.createTextNode("<cubes>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < cubes.length; i++)
	{
		for (var j = 0; j < cubes[i].length; j++)
		{
			output.appendChild(document.createTextNode("<cube row=\"" + cubes[i][j].row + "\" col=\"" + cubes[i][j].col + "\" xLength=\"" + cubes[i][j].xLength +
								   "\" zLength=\"" + cubes[i][j].zLength + "\" centerX=\"" + cubes[i][j].centerX + "\" centerY=\"" + cubes[i][j].centerY +
								   "\" centerZ=\"" + cubes[i][j].centerZ + "\" translateX=\"" + cubes[i][j].translateX + "\" translateY=\"" + cubes[i][j].translateY +
							           "\" translateZ=\"" + cubes[i][j].translateZ + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</cubes>"));
	output.appendChild(document.createElement("br"));
	
	output.appendChild(document.createTextNode("<crests>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < crests.length; i++)
	{
		for (var j = 0; j < crests[i].length; j++)
		{
			output.appendChild(document.createTextNode("<crest row=\"" + crests[i][j].row + "\" col=\"" + crests[i][j].col + "\" xLength=\"" + crests[i][j].xLength +
								   "\" zLength=\"" + crests[i][j].zLength + "\" centerX=\"" + crests[i][j].centerX + "\" centerY=\"" + crests[i][j].centerY +
								   "\" centerZ=\"" + crests[i][j].centerZ + "\" effect=\"" + crests[i][j].effect + "\" target=\"" + crests[i][j].target + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</crests>"));
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