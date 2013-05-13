window.addEventListener("load", init);

var ROWS = 25;
var COLS = 25;
var MAX_ROOMS = 50;
var MAX_TRANSLATE_X = 50;
var MAX_TRANSLATE_Y = 10;
var MAX_TRANSLATE_Z = 50;
var MAX_WALL_HEIGHT = 10;
var MAX_CUBE_HEIGHT = 10;
var MAX_CREST_HEIGHT = 10;

var isMouseDown = false;
var xmlString = "";

function init()
{
	window.addEventListener("mousedown", function(){isMouseDown = true;});
	window.addEventListener("mouseup", function(){isMouseDown = false;});
	
	var container = document.getElementById("container");
	var grid = document.getElementById("grid");
	
	var output = document.getElementById("output");
	output.addEventListener("change", loadXML);

	var wallHeight = document.getElementById("wallHeight");
	var cubeHeight = document.getElementById("cubeHeight");
	var crestHeight = document.getElementById("crestHeight");
	
	for (var i = 0; i < MAX_WALL_HEIGHT; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		wallHeight.appendChild(option);
	}
	
	for (var i = 0; i < MAX_CUBE_HEIGHT; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		cubeHeight.appendChild(option);
	}
	
	for (var i = 0; i < MAX_CREST_HEIGHT; i++)
	{
		var option = document.createElement("option");
		
		option.value = i + 1;
		option.appendChild(document.createTextNode(option.value));
		
		crestHeight.appendChild(option);
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
			td.setAttribute("style", "background-color: #452000; color: #FFFFFF;");
			td.appendChild(document.createTextNode("F"));
			td.appendChild(document.createComment("F|Floor"));
			td.addEventListener("mousedown", onTileClick);
			td.addEventListener("mouseover", onTileClick);
			td.addEventListener("mouseover", displayInfo);
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
	
	createXML();
}

function displayInfo()
{
	var info = document.getElementById("info");
	
	info.innerHTML = "";
	
	if (this.lastChild)
		info.innerHTML += "Tile Info <br/>" + this.lastChild.nodeValue;
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
	
	this.innerHTML = "";
	
	switch (selected.value)
	{
		case "wall":
			this.setAttribute("style", "background-color: #FF0000; color: #FFFFFF;");
			this.appendChild(document.createTextNode("W|" + document.getElementById("wallHeight").value));
			this.appendChild(document.createComment("W|" + document.getElementById("wallHeight").value + "|" + document.getElementById("textureType").value));
			break;
		case "floor":
			this.setAttribute("style", "background-color: #452000; color: #FFFFFF;");
			this.appendChild(document.createTextNode("F"));
			this.appendChild(document.createComment("F|" + document.getElementById("textureType").value));
			break;
		case "spawn":
			this.setAttribute("style", "background-color: #0000FF; color: #FFFFFF;");
			this.appendChild(document.createTextNode("S"));
			this.appendChild(document.createComment("Spawn " + document.getElementById("spawnDirection").value));
			break;
		case "exit":
			this.setAttribute("style", "background-color: #FFFF00; color: #000000;");
			this.appendChild(document.createTextNode("l"));
			this.appendChild(document.createComment("level" + document.getElementById("exitNum").value + ".xml"));
			break;
		case "cube":
			this.setAttribute("style", "background-color: #862125; color: #FFFFFF;");
			this.appendChild(document.createTextNode("c" + document.getElementById("cubeHeight").value));
			this.appendChild(document.createComment("c|" + document.getElementById("cubeHeight").value + "|" + document.getElementById("ucTranslateX").value +
								  "|" + document.getElementById("ucTranslateY").value + "|" + document.getElementById("ucTranslateZ").value +
								  "|" + document.getElementById("cubeFloor").checked + "|" + document.getElementById("textureType").value));
			break;
		case "crest":
			this.setAttribute("style", "background-color: #42CD76; color: #000000;");
			this.appendChild(document.createTextNode("C"));
			
			var hadesTypes = document.getElementsByName("hadesType");
			var selectedHades;
			
			for (var i = 0; i < hadesTypes.length; i++)
			{
				if (hadesTypes[i].checked)
					selectedHades = hadesTypes[i].value;
			}
			
			this.appendChild(document.createComment("Crest:" + document.getElementById("crestName").value + ":" + document.getElementById("unlockCube").value + 
								 ":" + document.getElementById("crestDirection").value + ":" + document.getElementById("crestFloor").checked + ":" + selectedHades +
								 ":" + document.getElementById("crestHeight").value));
			break;
		default:
			this.setAttribute("style", "background-color: #FFFFFF; color: #000000;");
			break;
	}
	
	createXML();
}

function loadXML()
{
	var xmlParser = new DOMParser();
	var xmlDoc = xmlParser.parseFromString(this.value, "text/xml");
	
	var gridElements = document.getElementsByClassName("gridElement");
	
	var floors = xmlDoc.getElementsByTagName("floor");
	var walls = xmlDoc.getElementsByTagName("wall");
	var spawns = xmlDoc.getElementsByTagName("spawn");
	var exits = xmlDoc.getElementsByTagName("exit");
	var cubes = xmlDoc.getElementsByTagName("cube");
	var crests = xmlDoc.getElementsByTagName("crest");
	
	for (var i = 0; i < gridElements.length; i++)
	{
		gridElements[i].innerHTML = "";
		gridElements[i].setAttribute("style", "background-color: #FFFFFF; color: #000000;");
	}
	
	for (var i = 0; i < floors.length; i++)
	{
		var row = parseInt(floors[i].getAttribute("row"));
		var col = parseInt(floors[i].getAttribute("col"));
		var xLength = parseInt(floors[i].getAttribute("xLength"));
		var zLength = parseInt(floors[i].getAttribute("zLength"));
		var centerX = parseFloat(floors[i].getAttribute("centerX"));
		var centerY = parseFloat(floors[i].getAttribute("centerY"));
		var centerZ = parseFloat(floors[i].getAttribute("centerZ"));
		var texture = floors[i].getAttribute("texture");
		
		for (var j = 0; j < xLength; j++)
		{
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #452000; color: #FFFFFF;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("F"));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment("F|" + texture));
			
			for (var k = 1; k < zLength; k++)
			{
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #452000; color: #FFFFFF;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("F"));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment("F|" + texture));
			}
		}	
	}
	
	for (var i = 0; i < walls.length; i++)
	{
		var row = parseInt(walls[i].getAttribute("row"));
		var col = parseInt(walls[i].getAttribute("col"));
		var xLength = parseInt(walls[i].getAttribute("xLength"));
		var yLength = parseInt(walls[i].getAttribute("yLength"));
		var zLength = parseInt(walls[i].getAttribute("zLength"));
		var texture = walls[i].getAttribute("texture");
	
		for (var j = 0; j < xLength; j++)
		{
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].innerHTML = "";
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #FF0000; color: #FFFFFF;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("W|" + yLength));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment("W|" + yLength + "|" + texture));
			
			for (var k = 1; k < zLength; k++)
			{
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].innerHTML = "";
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #FF0000; color: #FFFFFF;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("W|" + yLength));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment("W|" + yLength + "|" + texture));
			}
		}	
	}
	
	for (var i = 0; i < spawns.length; i++)
	{
		var row = parseInt(spawns[i].getAttribute("row"));
		var col = parseInt(spawns[i].getAttribute("col"));
		var xLength = parseInt(spawns[i].getAttribute("xLength"));
		var zLength = parseInt(spawns[i].getAttribute("zLength"));
		var dir = spawns[i].getAttribute("dir");
	
		for (var j = 0; j < xLength; j++)
		{
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].innerHTML = "";
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #0000FF; color: #FFFFFF;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("S"));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment("Spawn " + dir));
			
			for (var k = 1; k < zLength; k++)
			{
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].innerHTML = "";
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #0000FF; color: #FFFFFF;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("S"));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment("Spawn " + dir));
			}
		}	
	}
	
	for (var i = 0; i < exits.length; i++)
	{
		var row = parseInt(exits[i].getAttribute("row"));
		var col = parseInt(exits[i].getAttribute("col"));
		var xLength = parseInt(exits[i].getAttribute("xLength"));
		var zLength = parseInt(exits[i].getAttribute("zLength"));
		var file = exits[i].getAttribute("file");
	
		for (var j = 0; j < xLength; j++)
		{
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].innerHTML = "";
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #FFFF00; color: #000000;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("l"));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment(file));
			
			for (var k = 1; k < zLength; k++)
			{
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].innerHTML = "";
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #FFFF00; color: #000000;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("l"));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment(file));
			}
		}	
	}
	
	var cubeObjs = new Array();
	
	for (var i = 0; i < cubes.length; i++)
	{
		var row = parseInt(cubes[i].getAttribute("row"));
		var col = parseInt(cubes[i].getAttribute("col"));
		var xLength = parseInt(cubes[i].getAttribute("xLength"));
		var yLength = parseInt(cubes[i].getAttribute("yLength"));
		var zLength = parseInt(cubes[i].getAttribute("zLength"));
		var translateX = parseInt(cubes[i].getAttribute("translateX"));
		var translateY = parseInt(cubes[i].getAttribute("translateY"));
		var translateZ = parseInt(cubes[i].getAttribute("translateZ"));
		var texture = cubes[i].getAttribute("texture");
		var cubeFloor;

		cubeObjs.push({row: row, col: col, yLength: yLength, translateX: translateX, translateY: translateY, translateZ: translateZ});
		
		for (var j = 0; j < xLength; j++)
		{		
			if ((gridElements[((ROWS - row - 1) * ROWS) + (col + j)].firstChild != undefined) && (gridElements[((ROWS - row - 1) * ROWS) + (col + j)].firstChild.nodeValue == "F"))
				cubeFloor = "true";
			else
				cubeFloor = "false";
			
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].innerHTML = "";
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #862125; color: #FFFFFF;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("c" + yLength));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment("c|" + yLength + "|" + translateX + "|" + translateY + "|" + translateZ +
														"|" + cubeFloor + "|" + texture));
			
			for (var k = 1; k < zLength; k++)
			{
				if ((gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].firstChild != null) && (gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].firstChild.nodeValue == "F"))
					cubeFloor = "true";
				else
					cubeFloor = "false";
				
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].innerHTML = "";
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #862125; color: #FFFFFF;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("c" + yLength));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment("c|" + yLength + "|" + translateX + "|" + translateY + "|" + translateZ +
															     "|" + cubeFloor + "|" + texture));
			}
		}	
	}
	
	cubeObjs = combineElements(cubeObjs);
	listCubes(cubeObjs);
	
	for (var i = 0; i < crests.length; i++)
	{
		var row = parseInt(crests[i].getAttribute("row"));
		var col = parseInt(crests[i].getAttribute("col"));
		var xLength = parseInt(crests[i].getAttribute("xLength"));
		var yLength = parseInt(crests[i].getAttribute("yLength"));
		var zLength = parseInt(crests[i].getAttribute("zLength"));
		var dir = crests[i].getAttribute("dir");
		var effect = parseInt(crests[i].getAttribute("effect"));
		var target = crests[i].getAttribute("target");
		var placement = crests[i].getAttribute("placement");
		var crestFloor;
	
		for (var j = 0; j < xLength; j++)
		{
			if ((gridElements[((ROWS - row - 1) * ROWS) + (col + j)].firstChild != undefined) && (gridElements[((ROWS - row - 1) * ROWS) + (col + j)].firstChild.nodeValue == "F"))
				crestFloor = "true";
			else
				crestFloor = "false";
			
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].innerHTML = "";
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].setAttribute("style", "background-color: #42CD76; color: #000000;");
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createTextNode("C"));
			gridElements[((ROWS - row - 1) * ROWS) + (col + j)].appendChild(document.createComment("Crest:" + effect + ":" + target + ":" + dir + ":"  + crestFloor + ":" + placement + ":" + yLength));
			
			for (var k = 1; k < zLength; k++)
			{
				if ((gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].firstChild != null) && (gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].firstChild.nodeValue == "F"))
					crestFloor = "true";
				else
					crestFloor = "false";
				
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].innerHTML = "";
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].setAttribute("style", "background-color: #42CD76; color: #000000;");
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createTextNode("C"));
				gridElements[(((ROWS - row - 1) - k) * ROWS) + (col + j)].appendChild(document.createComment("Crest:" + effect + ":" + target + ":" + dir + ":" + crestFloor + ":" + placement + ":" + yLength));
			}
		}
	}
	
	createXML();
}

function createXML()
{
	var grid = document.getElementById("grid");
	var walls = new Array();
	var floors = new Array();
	var spawns = new Array();
	var exits = new Array();
	var cubes = new Array();
	var crests = new Array();
	
	for (var i = 0; i < ROWS; i++)
	{
		for (var j = 0; j < COLS; j++)
		{
			if (grid.childNodes[i].childNodes[j].lastChild)
			{	
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "W")
				{
					var strArray = grid.childNodes[i].childNodes[j].lastChild.nodeValue.split("|");
					
					walls.push({row: ((ROWS - i) - 1), col: j, yLength: strArray[1], texture: strArray[2]});
				}
				
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "F")
				{
					var strArray = grid.childNodes[i].childNodes[j].lastChild.nodeValue.split("|");
					
					floors.push({row: ((ROWS - i) - 1), col: j, texture: strArray[1]});
				}
				
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "S")
				{
					spawns.push({row: ((ROWS - i) - 1), col: j, dir: grid.childNodes[i].childNodes[j].lastChild.nodeValue.substr(6)});
					
					var floorTexture = "Floor";
					
					if (floors.length > 0)
						floorTexture = floors[floors.length - 1].texture;
					
					floors.push({row: ((ROWS - i) - 1), col: j, texture: floorTexture});
				}
				
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "l")
				{
					var floorTexture = "Floor";
					
					if (floors.length > 0)
						floorTexture = floors[floors.length - 1].texture;
					
					
					exits.push({row: ((ROWS - i) - 1), col: j, file: grid.childNodes[i].childNodes[j].lastChild.nodeValue});
					floors.push({row: ((ROWS - i) - 1), col: j, texture: floorTexture});
				}
				
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "c")
				{
					var strArray = grid.childNodes[i].childNodes[j].lastChild.nodeValue.split("|");
					
					cubes.push({row: ((ROWS - i) - 1), col: j, yLength: strArray[1], translateX: strArray[2], translateY: strArray[3], translateZ: strArray[4], texture: strArray[6]});
					
					var floorTexture = "Floor";
					
					if (floors.length > 0)
						floorTexture = floors[floors.length - 1].texture;
					
					if (strArray[5] == "true")
						floors.push({row: ((ROWS - i) - 1), col: j, texture: floorTexture});
				}
					
				if (grid.childNodes[i].childNodes[j].lastChild.nodeValue[0] == "C")
				{
					var strArray = grid.childNodes[i].childNodes[j].lastChild.nodeValue.split(":");
					var targetCube;
					var placementType;
					
					if (strArray[1] == 3)
					{
						targetCube = strArray[2];
						placementType = "";
					}
					else if (strArray[1] == 4)
					{
						targetCube = "";
						placementType = strArray[5];
					}
					else
					{
						targetCube = "";
						placementType = "";
					}	
					
					var floorTexture = "Floor";
					
					if (floors.length > 0)
						floorTexture = floors[floors.length - 1].texture;
					
					crests.push({row: ((ROWS - i) - 1), col: j, effect: strArray[1], target: targetCube, dir: strArray[3], placement: placementType, yLength: strArray[6]});
					
					if (strArray[4] == "true")
						floors.push({row: ((ROWS - i) - 1), col: j, texture: floorTexture});
				}
			}
		}
	}
		
	walls = combineElements(walls);
	floors = combineElements(floors);
	spawns = combineElements(spawns);
	exits = combineElements(exits);
	cubes = combineElements(cubes);
	crests = combineElements(crests);
	
	listCubes(cubes);
	writeXML(walls, floors, spawns, exits, cubes, crests);
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
				dir: undefined, file: undefined, effect: undefined, target: undefined, placement: undefined, texture: undefined};

		tempWall.row = elementArray[i].row;
		tempWall.col = elementArray[i].col;
		tempWall.yLength = elementArray[i].yLength;
		tempWall.dir = elementArray[i].dir;
		tempWall.file = elementArray[i].file;
		tempWall.effect = elementArray[i].effect;
		tempWall.target = elementArray[i].target;
		tempWall.placement = elementArray[i].placement;
		tempWall.centerX = tempWall.col + tempWall.xLength / 2;
		tempWall.centerZ = tempWall.row + tempWall.zLength / 2;
		tempWall.translateX = elementArray[i].translateX;
		tempWall.translateY = elementArray[i].translateY;
		tempWall.translateZ = elementArray[i].translateZ;
		tempWall.texture = elementArray[i].texture;
		
		wallRowCol[elementArray[i].row].push(tempWall);
	}
	
	// Combine rows
	for (var i = 0; i < wallRowCol.length; i++)
	{	
		for (var j = 0; j < wallRowCol[i].length - 1; j++)
		{	
			if ((wallRowCol[i].length > 0) && ((wallRowCol[i][j].col + wallRowCol[i][j].xLength) == wallRowCol[i][j + 1].col))
			{
				
				if (((wallRowCol[i][j].dir == undefined) || (((wallRowCol[i][j].dir != undefined) && (wallRowCol[i][j].dir == wallRowCol[i][j + 1].dir)))) &&
				   ((wallRowCol[i][j].file == undefined) || (((wallRowCol[i][j].file != undefined) && (wallRowCol[i][j].file == wallRowCol[i][j + 1].file)))) &&
				   ((wallRowCol[i][j].effect == undefined) || (((wallRowCol[i][j].effect == 4 && wallRowCol[i][j + 1].effect == 4)))) &&
				   ((wallRowCol[i][j].placement == undefined) || (((wallRowCol[i][j].placement != undefined) && (wallRowCol[i][j].placement == wallRowCol[i][j + 1].placement)))) &&
				   ((wallRowCol[i][j].texture == undefined) || (((wallRowCol[i][j].texture != undefined) && (wallRowCol[i][j].texture == wallRowCol[i][j + 1].texture)))) &&
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
					    (wallRowCol[i][l].col == wallRowCol[i + j][k].col) && (wallRowCol[i][l].xLength == wallRowCol[i + j][k].xLength))
					{
						
						if (((wallRowCol[i][l].dir == undefined) || (((wallRowCol[i][l].dir != undefined) && (wallRowCol[i][l].dir == wallRowCol[i + j][k].dir)))) &&
						   ((wallRowCol[i][l].file == undefined) || (((wallRowCol[i][l].file != undefined) && (wallRowCol[i][l].file == wallRowCol[i + j][k].file)))) &&
						   ((wallRowCol[i][l].effect == undefined) || (((wallRowCol[i][l].effect == 4 && wallRowCol[i + j][k].effect == 4)))) &&
						   ((wallRowCol[i][l].placement == undefined) || (((wallRowCol[i][l].placement != undefined) && (wallRowCol[i][l].placement == wallRowCol[i + j][k].placement)))) &&
						   ((wallRowCol[i][l].texture == undefined) || (((wallRowCol[i][l].texture != undefined) && (wallRowCol[i][l].texture == wallRowCol[i + j][k].texture)))) &&
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

function writeXML(walls, floors, spawns, exits, cubes, crests)
{
	var output = document.getElementById("output");
	
	xmlString = "<level>\n<walls>";
	
	for (var i = 0; i < walls.length; i++)
	{
		for (var j = 0; j < walls[i].length; j++)
		{
			xmlString += "\n<wall row=\"" + walls[i][j].row + "\" col=\"" + walls[i][j].col + "\" xLength=\"" + walls[i][j].xLength +  "\" yLength=\"" + walls[i][j].yLength +
				     "\" zLength=\"" + walls[i][j].zLength + "\" centerX=\"" + walls[i][j].centerX + "\" centerY=\"" + walls[i][j].centerY +
				     "\" centerZ=\"" + walls[i][j].centerZ + "\" texture=\"" + walls[i][j].texture + "\"/>";
		}
	}
	
	xmlString += "\n</walls>\n<floors>";
	
	for (var i = 0; i < floors.length; i++)
	{
		for (var j = 0; j < floors[i].length; j++)
		{
			xmlString += "\n<floor row=\"" + floors[i][j].row + "\" col=\"" + floors[i][j].col + "\" xLength=\"" + floors[i][j].xLength + "\" zLength=\"" + floors[i][j].zLength +
				     "\" centerX=\"" + floors[i][j].centerX + "\" centerY=\"" + floors[i][j].centerY + "\" centerZ=\"" + floors[i][j].centerZ +
				     "\" texture=\"" + floors[i][j].texture + "\"/>";
		}
	}
	
	xmlString += "\n</floors>\n<spawns>";	
	
	for (var i = 0; i < spawns.length; i++)
	{
		for (var j = 0; j < spawns[i].length; j++)
		{
			xmlString += "\n<spawn row=\"" + spawns[i][j].row + "\" col=\"" + spawns[i][j].col + "\" xLength=\"" + spawns[i][j].xLength +
				     "\" zLength=\"" + spawns[i][j].zLength + "\" centerX=\"" + spawns[i][j].centerX + "\" centerY=\"" + spawns[i][j].centerY +
				     "\" centerZ=\"" + spawns[i][j].centerZ + "\" dir=\"" + spawns[i][j].dir + "\"/>";
		}
	}
	
	xmlString += "\n</spawns>\n<exits>";
	
	for (var i = 0; i < exits.length; i++)
	{
		for (var j = 0; j < exits[i].length; j++)
		{
			xmlString += "\n<exit row=\"" + exits[i][j].row + "\" col=\"" + exits[i][j].col + "\" xLength=\"" + exits[i][j].xLength +
				     "\" zLength=\"" + exits[i][j].zLength + "\" centerX=\"" + exits[i][j].centerX + "\" centerY=\"" + exits[i][j].centerY +
				     "\" centerZ=\"" + exits[i][j].centerZ + "\" file=\"" + exits[i][j].file + "\"/>";
		}
	}
	
	
	xmlString += "\n</exits>\n<cubes>";
	
	for (var i = 0; i < cubes.length; i++)
	{	
		for (var j = 0; j < cubes[i].length; j++)
		{
			xmlString += "\n<cube row=\"" + cubes[i][j].row + "\" col=\"" + cubes[i][j].col + "\" xLength=\"" + cubes[i][j].xLength + "\" yLength=\"" + cubes[i][j].yLength +
				     "\" zLength=\"" + cubes[i][j].zLength + "\" centerX=\"" + cubes[i][j].centerX + "\" centerY=\"" + cubes[i][j].centerY +
				     "\" centerZ=\"" + cubes[i][j].centerZ + "\" translateX=\"" + cubes[i][j].translateX + "\" translateY=\"" + cubes[i][j].translateY +
				     "\" translateZ=\"" + cubes[i][j].translateZ + "\" texture=\"" + cubes[i][j].texture + "\"/>";
		}
	}
	
	xmlString += "\n</cubes>\n<crests>";
	
	for (var i = 0; i < crests.length; i++)
	{	
		for (var j = 0; j < crests[i].length; j++)
		{
			xmlString += "\n<crest row=\"" + crests[i][j].row + "\" col=\"" + crests[i][j].col + "\" xLength=\"" + crests[i][j].xLength + "\" yLength=\"" + crests[i][j].yLength +
				     "\" zLength=\"" + crests[i][j].zLength + "\" centerX=\"" + crests[i][j].centerX + "\" centerY=\"" + crests[i][j].centerY +
				     "\" centerZ=\"" + crests[i][j].centerZ + "\" effect=\"" + crests[i][j].effect + "\" target=\"" + crests[i][j].target +
				     "\" dir=\"" + crests[i][j].dir + "\" placement=\"" + crests[i][j].placement + "\"/>";
		}
	}
	
	xmlString += "\n</crests>\n</level>";
	
	output.value = xmlString;
	
	/*while (output.childNodes.length > 0)
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
	output.appendChild(document.createTextNode("<floors>"));
	output.appendChild(document.createElement("br"));
	
	for (var i = 0; i < floors.length; i++)
	{
		for (var j = 0; j < floors[i].length; j++)
		{
			output.appendChild(document.createTextNode("<floor row=\"" + floors[i][j].row + "\" col=\"" + floors[i][j].col + "\" xLength=\"" + floors[i][j].xLength + 
								   "\" zLength=\"" + floors[i][j].zLength + "\" centerX=\"" + floors[i][j].centerX + "\" centerY=\"" + floors[i][j].centerY +
								   "\" centerZ=\"" + floors[i][j].centerZ + "\"/>"));
			output.appendChild(document.createElement("br"));
		}
	}
	
	output.appendChild(document.createTextNode("</floors>"));
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
			
			console.log("HERE");
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
			output.appendChild(document.createTextNode("<cube row=\"" + cubes[i][j].row + "\" col=\"" + cubes[i][j].col + "\" xLength=\"" + cubes[i][j].xLength + "\" yLength=\"" + cubes[i][j].yLength +
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
	
	output.appendChild(document.createTextNode("</level>"));*/
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