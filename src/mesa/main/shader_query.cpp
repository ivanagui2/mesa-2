/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file shader_query.cpp
 * C-to-C++ bridge functions to query GLSL shader data
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "main/core.h"
#include "glsl_symbol_table.h"
#include "ir.h"
#include "shaderobj.h"

extern "C" {
#include "shaderapi.h"
}

GLint GLAPIENTRY
_mesa_GetAttribLocationARB(GLhandleARB program, const GLcharARB * name)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *const shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetAttribLocation");

   if (!shProg) {
      return -1;
   }

   if (!shProg->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetAttribLocation(program not linked)");
      return -1;
   }

   if (!name)
      return -1;

   /* Not having a vertex shader is not an error.
    */
   if (shProg->_LinkedShaders[MESA_SHADER_VERTEX] == NULL)
      return -1;

   exec_list *ir = shProg->_LinkedShaders[MESA_SHADER_VERTEX]->ir;
   foreach_list(node, ir) {
      const ir_variable *const var = ((ir_instruction *) node)->as_variable();

      /* The extra check against VERT_ATTRIB_GENERIC0 is because
       * glGetAttribLocation cannot be used on "conventional" attributes.
       *
       * From page 95 of the OpenGL 3.0 spec:
       *
       *     "If name is not an active attribute, if name is a conventional
       *     attribute, or if an error occurs, -1 will be returned."
       */
      if (var == NULL
	  || var->mode != ir_var_in
	  || var->location == -1
	  || var->location < VERT_ATTRIB_GENERIC0)
	 continue;

      if (strcmp(var->name, name) == 0)
	 return var->location - VERT_ATTRIB_GENERIC0;
   }

   return -1;
}