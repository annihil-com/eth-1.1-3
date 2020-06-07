quakeABlueShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadblue.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}
quakeACyanShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadcyan.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}
quakeAGreenShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadgreen.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}
quakeARedShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadred.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}
quakeATealShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadteal.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}
quakeAYellowShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		{
			map gfx/effects/quadyellow.tga
			blendfunc GL_ONE GL_ONE
				tcGen environment
				tcmod rotate 15
				tcmod scroll 1 .1
		}
	}

quakeBBlueShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadblue.tga
			tcGen environment
		}
	}
quakeBCyanShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadcyan.tga
			tcGen environment
		}
	}
quakeBGreenShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadgreen.tga
			tcGen environment
		}
	}
quakeBRedShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadred.tga
			tcGen environment
		}
	}
quakeBTealShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadteal.tga
			tcGen environment
		}
	}
quakeBYellowShader
	{
		q3map_surfacelight 1800
		deformVertexes wave 100 sin 2 0 0 0
		nofog
		{
			map gfx/effects/quadyellow.tga
			tcGen environment
		}
	}

quakeCBlueShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadblue.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}
quakeCCyanShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadcyan.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}
quakeCGreenShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadgreen.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}
quakeCRedShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadred.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}
quakeCTealShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadteal.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}
quakeCYellowShader
	{
		q3map_surfacelight 1800
		{
			map textures/effects/envmap_slate.tga
			rgbGen lightingDiffuse
			tcGen environment
		}
		cull none
		{
			map gfx/effects/quadyellow.tga
			tcmod rotate 2
			tcmod scroll 1 .01
			tcGen environment
			rgbGen lightingDiffuse
		}
	}

glowBlueShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_blue.tga
			blendfunc GL_ONE GL_ONE
		}
	}
glowCyanShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_teal.tga
			blendfunc GL_ONE GL_ONE
		}
	}
glowGreenShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_grn.tga
			blendfunc GL_ONE GL_ONE
		}
	}
glowRedShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_red.tga
			blendfunc GL_ONE GL_ONE
		}
	}
glowTealShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_cyan.tga
			blendfunc GL_ONE GL_ONE
		}
	}
glowYellowShader
	{
		deformVertexes wave 100 sin 4 0 0 0
		{
			map menu/art/fx_yel.tga
			blendfunc GL_ONE GL_ONE
		}
	}

flatBlueShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/blue.tga
		}
	}
flatCyanShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/cyan.tga
		}
	}
flatGreenShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/green.tga
		}
	}
flatRedShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/red.tga
		}
	}
flatTealShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/teal.tga
		}
	}
flatYellowShader
	{
		cull back
		surfaceparm nodlight
		surfaceparm nolightmap
		sort 3
		{
			map gfx/colors/yellow.tga
		}
	}

flatWhiteShader
{
	cull back
	surfaceparm nodlight
	surfaceparm nolightmap
	sort 3
	{
		map menu/art/fx_white.tga
	}
}
friendMortarShader
{
	q3map_surfacelight 1800
	deformVertexes wave 100 sin 40 0 0 0
	{
		map gfx/effects/quadblue.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
		tcmod rotate 15
		tcmod scroll 1 .1
	}
}
enemyMortarShader
{
	q3map_surfacelight 1800
	deformVertexes wave 100 sin 40 0 0 0
	{
		map gfx/effects/quadred.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
		tcmod rotate 15
		tcmod scroll 1 .1
	}
}
