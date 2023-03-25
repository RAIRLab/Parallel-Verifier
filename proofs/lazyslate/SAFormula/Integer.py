from .Formula import Formula

class Integer(Formula):

  def __init__(self, fstring, justification, val):
    self.val = int(val)
    super().__init__(fstring, justification)



  @classmethod
  def from_string(cls, fstring, justification=None):
    val = int(fstring) 
    return cls(fstring, justification, val)


  
  @classmethod
  def from_args(cls, arg, justification=None):
    fstring = str(arg)
    val = int(arg)
    return cls(fstring, justification, val)



  def __str__(self):
    return str(self.val)



  def is_annotated(self):
    return False
