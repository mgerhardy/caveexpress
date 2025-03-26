textures = {{% for sprite in allSprites %}
	["{{sprite.trimmedName}}"] = {
		image = "{{texture.trimmedName}}",
		x0 = {{sprite.frameRectRel.x}},
		y0 = {{sprite.frameRectRel.y}},
		x1 = {{sprite.frameRectRel.width}},
		y1 = {{sprite.frameRectRel.height}},
		trimmedoffsetx = {{sprite.cornerOffset.x}},
		trimmedoffsety = {{sprite.cornerOffset.y}},
		trimmedwidth = {{sprite.sourceRect.width}},
		trimmedheight = {{sprite.sourceRect.height}},
		untrimmedwidth = {{sprite.untrimmedSize.width}},
		untrimmedheight = {{sprite.untrimmedSize.height}},
	},{% endfor %}
}
