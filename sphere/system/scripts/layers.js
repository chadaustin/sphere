var gLayerTileDetails = new Array();

function ConvertLayer(layer_index, tile_index, dont_remember_details) {
  // init
  if (!dont_remember_details) {
    gLayerTileDetails[layer_index] = new Object();
    gLayerTileDetails[layer_index].tile_index = tile_index;
    gLayerTileDetails[layer_index].tiles = new Array(GetLayerWidth(layer_index));
    for (var i = 0; i < GetLayerWidth(layer_index); ++i)
      gLayerTileDetails[layer_index].tiles[i] = new Array(GetLayerHeight(layer_index));
  }

  // store and change tile
  for (var j = 0; j < GetLayerWidth(layer_index); ++j)
  {
    for (var k = 0; k < GetLayerHeight(layer_index); ++k) {
      if (!dont_remember_details)
        gLayerTileDetails[layer_index].tiles[j][k] = GetTile(j, k, layer_index);
      SetTile(j, k, layer_index, tile_index);
    }
  }
  
}

function ClearLayerTileDetails(layer_index) {
  gLayerTileDetails[layer_index] = new Object();
  gLayerTileDetails[layer_index].tiles = undefined;
  gLayerTileDetails[layer_index].tile_index = undefined;
}

function RevertLayer(layer_index) {
  for (var j = 0; j < GetLayerWidth(layer_index); ++j)
  {
    for (var k = 0; k < GetLayerHeight(layer_index); ++k) {
      SetTile(j, k, layer_index, gLayerTileDetails[layer_index].tiles[j][k]);
    }
  }
  ClearLayerTileDetails(layer_index);
}

function IsLayerConverted(layer_index) {
  return (gLayerTileDetails[layer_index] != undefined && gLayerTileDetails[layer_index].tile_index != undefined);
}

function ToggleLayer(layer_index, tile_index) {
  if (IsLayerConverted(layer_index))
    RevertLayer(layer_index);
  else
    ConvertLayer(layer_index, tile_index, false);
}