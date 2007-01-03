/**
 * named_parameters.js
 *
 * Pass parameters to functions by name, in any order.
 * Handy for more readable code.
 *
 * API:
 * Function.setParameters(in_params, required_template[, optional_template])
 * Object.mergeStrict(other_object)
 * Object.mergeLoose(other_object)
 * 
 * Exceptions:
 * MissingArgumentException
 * IllegalArgumentException
 * BadMergeException
 *
 * Usage:
 * In the function where you want named parameters:
 * 
 *   function drawText(params) {
 *     var data = Function.setParameters(params, {text: ""}, {title: "", position: {x: 2, y: 2}});
 *     data.position.mergeStrict(params.position);
 *     // ...
 *     // Code that uses data.text, data.title, data.position.x, data.position.y
 *   }
 * 
 * Then call it in any of these ways. Note the use of the literal object:
 * 
 *   drawText({text: "Hey guys, what's going on?"});
 *   drawText({title: "Bobby", text: "Nothing much."});
 *   drawText({text: "Hahahahaha!", position: {x: 5, y: 20}});
 * 
 * All functions ignore nested object parameters, so you have to set them
 * manually using Object.mergeStrict() or Object.mergeLoose(), as shown
 * in the example above.
 * 
 * N.B.: Any Sphere objects will still have to be checked manually:
 * 
 *   function drawWindow(params) {
 *     // Stuff from above
 *     // ...
 *     if (!params.windowStyle
 *         || typeof params.windowStyle != "[object windowstyle]")
 *       throw "IllegalArgumentException: blah blah blah...";
 *     data.windowStyle = params.windowStyle;
 *     // ...
 *     // Code that uses data.windowStyle, etc.
 *   }
 *
 * @author	Tung Nguyen
 */

/**
 * Checks parameters of an input object against given templates,
 * and merges them into a single object. Ignores object members.
 * 
 * @param	in_params	Object containing named input parameters.
 * @param	required_template	A template object whose members must
 * 	exist in the in_params object.
 * @param	optional_template	A template object whose members may be
 * 	overridden by those in the in_params object.
 * @throws	MissingArgumentException	in_params does not have a
 * 	member that required_template needs.
 * @throws	IllegalArgumentException	in_params has a member that
 * 	doesn't exist in either template, or one that does not match types with
 *	the template requirements.
 * @return	A single object containing all members of the input objects,
 * 	with in_param's members overriding defaults as needed.
 */
Function.prototype.setParameters = function (in_params, required_template, optional_template)
{
	if (!in_params)
		throw "MissingArgumentException: Function.setParameters() requires in_params "
				+ "argument.";
	if (!required_template)
		throw "MissingArgumentException: Function.setParameters() requires "
				+ "required_template argument.";
	
	// Fill in default values
	var data = {};
	for (var r in required_template)
	{
		if (typeof required_template[r] != "object"
				&& (typeof required_template[r]).indexOf("[object") != 0)
		{
			if (r in in_params == false)
				throw "MissingArgumentException: Function.setParameters() requires "
						+ "in_params to have member \"" + r +"\".";
			data[r] = required_template[r];
		}
	}
	for (var o in optional_template)
	{
		if (typeof required_template[o] != "object"
				&& (typeof required_template[o]).indexOf("[object") != 0)
		{
			data[o] = optional_template[o];
		}
	}
	
	for (var i in in_params)
	{
		if (typeof in_params[i] != "object"
				&& (typeof in_params[i]).indexOf("[object") != 0)
		{
			if (i in required_template)
			{
				if (typeof in_params[i] == typeof required_template[i])
					data[i] = in_params[i];
				else
					throw "IllegalArgumentException: Function.setParameters() "
							+ "detected mismatched in_params/required_template "
							+ "member \"" + i + "\".";
			}
			else if (i in optional_template)
			{
				if (typeof in_params[i] == typeof optional_template[i])
					data[i] = in_params[i];
				else
					throw "IllegalArgumentException: Function.setParameters() "
							+ "detected mismatched in_params/optional_template "
							+ "member \"" + i + "\".";
			}
			else
			{
				throw "IllegalArgumentException: Function.setParameters() could not find "
						+ "a matching member \"" + i + "\" in required/optional_template "
						+ "from in_params.";
			}
		}
		else
		{
			if (i in required_template)
			{
				if (typeof in_params[i] != typeof required_template[i])
					throw "IllegalArgumentException: Function.setParameters() "
							+ "detected mismatched in_params/required_template "
							+ "member \"" + i + "\".";
			}
			else if (i in optional_template)
			{
				if (typeof in_params[i] != typeof optional_template[i])
					throw "IllegalArgumentException: Function.setParameters() "
							+ "detected mismatched in_params/optional_template "
							+ "member \"" + i + "\".";
			}
			else
			{
				throw "IllegalArgumentException: Function.setParameters() could not find "
						+ "a matching member \"" + i + "\" in required/optional_template "
						+ "from in_params.";
			}
		}
	}
	
	return data;
}

/**
 * Takes an object with exact matching members, and adopts all of them.
 * Object members are ignored.
 *
 * @param	other_object	Object from which to take member data.
 * @throws	BadMergeException	One object doesn't have a member that
 * 	the other one needs.
 */
Object.prototype.mergeStrict = function (other_object)
{
	if (!other_object || typeof other_object != "object")
		throw "BadMergeException: Object.mergeStrict() requires an object input argument.";
	
	for (var o in other_object)
	{
		if (o in this == false)
			throw "BadMergeException: Object.mergeStrict() found unexpected member \""
					+ o + "\" in passed argument.";
	}
	
	for (var t in this)
	{
		if (t in other_object)
		{
			if (typeof this[t] != "object"
					&& (typeof this[t]).indexOf("[object") != 0)
				this[t] = other_object[t];
		}
		else
		{
			throw "BadMergeException: Object.mergeStrict() argument is missing required "
					+ "member \"" + t + "\".";
		}
	}
}

/**
 * Adopts the members of an input object, having its own members overridden as
 * needed. Object members are ignored.
 *
 * @param	other_object	Object from which to take member data.
 */
Object.prototype.mergeLoose = function (other_object)
{
	if (other_object)
	{
		for (var i in other_object)
		{
			if (typeof other_object[i] != "object"
					&& (typeof other_object[i]).indexOf("[object") != 0)
				this[i] = other_object[i];
		}
	}
}
