from abc import abstractmethod

from .Parser  import parse_fstring, parse_sexpr
from .Formula import Formula
from .Atom    import Atom

# fstring       -- String                -- String representation of formula (in s-expression style) (e.g. (Happy john))
# justification -- Justification || None -- Links the formula to its justification
# quant_vars    -- List(Atom)            -- List of variables (e.g. ["x"])
# formula       -- Formula               -- Sub-formula (e.g. "(Happy x)")
class Quantifier(Formula):

  def __init__(self, fstring, justification, quant_vars, formula):

    super().__init__(fstring, justification)
    self.quant_vars = quant_vars
    self.formula    = formula



  @classmethod
  def from_string(cls, fstring, justification=None):
    args = parse_sexpr(fstring) # Parse s-expression

    var_string = args[1]                                # e.g. "[?x ?y ?z]"
    var_string = var_string[1:len(var_string)-1]        # e.g. "?x ?y ?z"
    var_list   = var_string.replace('?', '').split(' ') # e.g. ["x", "y", "z"]

    quant_vars = []
    for v in var_list:
      quant_vars.append(Atom.from_string(v))

    formula = parse_fstring(args[2])

    return cls(fstring, justification, quant_vars, formula)



  # The sub-classes will need to define this on their own
  @classmethod
  @abstractmethod 
  def from_args(cls, args, justification=None):
    pass



  def is_annotated(self):
    return False



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other)



  def __hash__(self):
    return super().__hash__()

