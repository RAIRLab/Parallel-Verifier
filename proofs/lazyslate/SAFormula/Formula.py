from abc import ABC, abstractmethod

# fstring       -- String                -- String representation of formula (in s-expression style)
# justification -- Justification || None -- Links the formula to its justification
#                                           Sub-formulae and unproven formulae should have justification set to None
class Formula(ABC):

  def __init__(self, fstring, justification):
    self.fstring       = fstring
    self.justification = justification



  # Constructs a Formula object using a fstring
  #
  # Implementations should parse fstring to get object attributes then
  # pass fstring and the attributes to the class constructor
  @classmethod
  @abstractmethod
  def from_string(cls, fstring, justification):
    pass



  # Constructs a Formula object using arguments
  #
  # Implementations should take whichever arguments are appropriate,
  # use them to construct a well-formed fstring, then pass fstring and
  # the attributes to the class constructor
  @classmethod
  @abstractmethod
  def from_args(cls, args, justification):
    pass


  # Implementations should return True if the formula is annotated
  # (e.g. a strength-factor-annotated belief, or as in some of our
  # papers but yet to be implemented as of this writing, SF-annotated
  # knowledge), False otherwise.
  @abstractmethod
  def is_annotated(self):
    pass



  def get_justification(self):
    if(self.justification == None): return "NO JUSTIFICATION"
    return str(self.justification)



  def __str__(self):
    return self.fstring



  def __eq__(self, other):
    return isinstance(other, Formula) and self.fstring == other.fstring



  def __hash__(self):
    return hash(self.fstring)

