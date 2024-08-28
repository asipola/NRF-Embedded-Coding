entity <entity_name> is
generic (
<generic_name> : <type>
);
port (
<input_name> : in <type>;
<output_name> : out <type>;
<bidir_name> : inout <type>
);
end entity <entity_name>;

architecture <architecture_name> of <entity_name> is
-- signal declarations
signal <signal_name> : <signal_type>;
begin
-- functional RTl (or structural) code
end architecture <architecture_name>;