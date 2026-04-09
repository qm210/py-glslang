import unittest

from . import pyglsl

class ASTCase(unittest.TestCase):
    def test_parse_small_shader(self):
        given = """#version 450
        layout (location = 0) out vec4 outColor;
        layout (location = 2) uniform sampler2D iChannel0;
        void main()
        {
            outColor = vec4(vec3(0), 1.);
        }
        """
        target = pyglsl.parse(given, stage=pyglsl.Stage.FRAG)
        self.assertEqual(target.ok, True)


if __name__ == '__main__':
    unittest.main()
