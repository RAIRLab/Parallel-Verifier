from .Formula import Formula
from utils    import color_str

from SETTINGS import SPINDENT


# Formulae can be justified in three main ways (other than simply "given")
#
# (1) Using a ShadowAdjudicator inference schema
# (2) Using ShadowProver
# (3) Using both (i.e. to prove clauses of a formula involving both annotated beliefs and other modal formulae)


# isgiven       -- Boolean                          -- True if formula is assumed, false otherwise
# formula       -- List(Formula) || Formula || None -- Formula(e) used in proof (i.e. input to schema). 
# schema        -- String                   || None -- Inference schema used (If justification is solely via ShadowProver, this is set to None).
# sp_output     -- String                           -- If ShadowProver was used to prove any portion of the formula, this contains the output.
class Justification:

  # Last two parameters are None if (isgiven == True)
  def __init__(self, isgiven, formula=None, schema=None, sp_output=None):

    if(isgiven == False and schema == None and sp_output == None):
      print("ERROR: Justification must use at least one of (1) a SA inference schema or (2) a SP proof.")
      exit(1)

    self.isgiven       = isgiven
    self.formula       = formula
    self.schema        = schema
    self.sp_output     = sp_output



  # Returns a string form of the Justification object
  # with a given indent. If provided, indent is expected
  # to be a string of spaces.
  def string_with_indent(self, indent=""):

    if self.isgiven: return indent + "GIVEN\n"

    out = ""

    if not self.schema == None:
      out += indent + color_str("Applied '" + self.schema, 'blue')
      if self.formula == None:
        out += color_str("'.", 'blue') + "\n"
      else:
        out += color_str("' to: ", 'blue')
        if isinstance(self.formula, Formula):
          out += str(self.formula) + "\n"
        elif len(self.formula) == 0:
          out += "[]\n"
        else:
          for f in self.formula:
            out += str(f) + "; "
          out = out[:len(out)-2] # Remove trailing separator
          out += "\n"

    if not self.sp_output == None:
      out += indent + color_str("Proved via ShadowProver", 'blue')
      if self.formula == None:
        out += ":\n"
      else:
        out += " [and a sub-argument of "
        if isinstance(self.formula, Formula): out += str(self.formula) + "]:\n"
        else:
          for f in self.formula:
            out += str(f) + "; "
          out = out[:len(out)-2] # Remove trailing separator
          out += "]:\n"

      # Add ShadowProver output, with indents inserted
      out += indent + SPINDENT + self.sp_output.replace("\n", "\n"+indent+SPINDENT) + "\n"

    return out



  def __str__(self):
    return string_with_indent()
