/**
 * named_parameters.js
 *
 * Allow passing parameters to a function by name.
 * See docs/system_scripts/named_parameters.txt for more details.
 *
 * @author	tunginobi
 */

Function.prototype.setParameters = function (params, template, strict)
{
	function CopyObject(data_in, obj)
	{
		for (var o in obj)
		{
			if (typeof obj[o] === "object")
				CopyObject(data_in[o], obj[o]);
			else
				data_in[o] = obj[o];
		}
	}
	
	function CopyTemplate(data_in, tmp)
	{
		var req = tmp["req!"];
		var opt = tmp["opt!"];
		for (var r in req)
		{
			if (typeof req[r] === "object")
			{
				data_in[r] = {};
				CopyTemplate(data_in[r], req[r]);
			}
			else
				data_in[r] = req[r];
		}
		for (var o in opt)
		{
			if (typeof opt[o] === "object")
			{
				data_in[o] = {};
				CopyTemplate(data_in[o], opt[o]);
			}
			else
				data_in[o] = opt[o];
		}
	}
	
	if ("req!" in template === false)
		throw "InvalidTemplateException: Function.setParameters()"
				+ " could not find template member \"req!\".";
	if ("opt!" in template === false)
		throw "InvalidTemplateException: Function.setParameters()"
				+ " could not find template member \"opt!\".";
	
	// Set defaults
	var data = {};
	CopyTemplate(data, template);
	
	if (strict)
	{
		// Cross check params's members against the required ones
		for (var r in template["req!"])
		{
			if (r in params === false)
				throw "MissingArgumentException: Function.setParameters()"
						+ " could not find required argument \"" + r + "\".";
		}
		
		// Cross check params's members against all parameters and transfer.
		for (var p in params)
		{
			if (p in template["req!"] || p in template["opt!"])
			{
				var which = "opt!";
				if (p in template["req!"])
					which = "req!";
				if (typeof params[p] !== typeof template[which][p])
				{
					throw "IllegalArgumentException: Function.setParameters()"
							+ " type mismatch for argument \"" + p + "\";"
							+ " expected " + typeof template[which][p] + ","
							+ " found " + typeof params[p] + ".";
				}
				else if (typeof params[p] === "object")
					data[p] = Function.setParameters(params[p], template[which][p], strict);
				else
					data[p] = params[p];
			}
			else
			{
				throw "IllegalArgumentException: Function.setParameters()"
						+ " found stray argument \"" + p + "\".";
			}
		}
	}
	else
		CopyObject(data, params);
	
	return data;
}
