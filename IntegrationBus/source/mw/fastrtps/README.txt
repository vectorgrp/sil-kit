Developer Notes 
===============

Updating IDL
------------
- Install java for fastrtpsgen.
  apt install openjdk-11-jre	
- Update idl/target.idl.
- Remove generated files (.h,.cxx) before running fastrtpsgen

Example
~~~~~~~
- Updating a LIN struct. Some definitions might cause errors, such as:
	  LinTopics.idl:56:19: error: Illegal identifier: ib::sim::lin::idl::frame is
	    already defined (Type: ib::sim::lin::idl::Frame=com.eprosima.idl.parser.tree.TypeDeclaration@58a90037)
  This is a known bug -- we have a very old fastrtpsgen version, but some IDLs definitions predate it.
  Fix: add a "_" in front of the identifier, e.g.:
	  Old: Frame frame;
	  New: Frame _frame;
- delete old generated files:
	  rm LinTopics.{h,cxx}
	  rm LinTopicsPubSubTypes.{h,cxx}
- Run the bundled fastrtpsgen on the file: 
	  vib-main/ThirdParty/Fast-RTPS/fastrtpsgen/scripts/fastrtpsgen LinTopics.idl
- Update type conversions in VIB source tree, e.g. in this case:
	  vib-main/IntegrationBus/source/mw/fastrtps/IdlTypeConversionLin.hpp

