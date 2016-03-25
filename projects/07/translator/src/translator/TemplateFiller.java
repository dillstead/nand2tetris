package translator;

import java.util.Map;

class TemplateFiller 
{
	static String fillTemplate(String template, char startDelim, char endDelim, Map<String, String> params)
	{
		StringBuilder sb = new StringBuilder(template.length());
		
		int prevPos = 0;
		int startPos = template.indexOf(startDelim, prevPos);
		while (startPos != -1)
		{
			sb.append(template.substring(prevPos, startPos > 0 ? startPos - 1 : 0));
			int endPos = template.indexOf(endDelim, startPos);
			// check if empty
			sb.append(params.get(template.substring(startPos + 1, endPos - 1)));
			prevPos = endPos + 1;
			startPos = template.indexOf(startDelim, prevPos);
		}
		// check if empty
		sb.append(template.substring(prevPos));
		return null;
	}
}
