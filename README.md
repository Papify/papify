Papify
======
Papify is a tool that helps determine the performance of decoders developed with the [Open RVC-CAL Compiler (ORCC)](http://orcc.sourceforge.net/) using its C backend in order to point the developer into the right direction when it comes to optimization. It uses de [Performance Application Programming Interface (PAPI)](http://icl.cs.utk.edu/papi/), **which you need to have installed in the system where the code will be tested**. 

##Configuration
Papify requires its configuration to be added into the **xcf** file located inside the *src* directory of your ORCC generated code using the **C backend**. It must be added right after `</Instances>` and before `</Configuration>`. 

###Config examples
####Example 1
```
<Papi>
    <Events>
        <Event id="PAPI_TOT_INS"/>
        <Event id="PAPI_L1_DCM"/>
        <Event id="PAPI_TOT_CYC"/>
    </Events>
</Papi>
```

This is the minimun required input parameters. You will always have to specify a set of [PAPI events](http://icl.cs.utk.edu/projects/papi/presets.html). In this case, as no actors are specified, the events will be coded into every existing actor and action available, with the exception of "isSchedulable_" actions. When using this kind of config, one single event set will be used for all actions inside an actor.

The maximun amount of events you can add depends on your hardware. Papify won't verify this, but PAPI will output an error message if this is the case.


####Example 2
```
<Papi>
    <Events>
        <Event id="PAPI_TOT_INS"/>
        <Event id="PAPI_L1_DCM"/>
        <Event id="PAPI_TOT_CYC"/>
    </Events>
    <Instances>
        <Instance id="SomeActor"/> 
        <Instance id="SomeOtherActor"/>
    </Instances>
</Papi>
```

Here actors are specified, meaning that (in this example) only `SomeActor` and `SomeOtherActor` will receive PAPI code, and as no actios were specified, PAPI code will include them all with the exception of "isScheludable_" ones. As in the previous example, one single event set per actor is used. You can specify as many actors as you want.

####Example 3
```
<Papi>
    <Events>
        <Event id="PAPI_TOT_INS"/>
        <Event id="PAPI_L1_DCM"/>
        <Event id="PAPI_TOT_CYC"/>
    </Events>
    <Instances>
        <Instance id="SomeActor"> 
            <Action id="SomeAction"/>
            <Action id="SomeOtherAction"/>
        </Instance>
        <Instance id="SomeOtherActor"/>
    </Instances>
</Papi>
```
In this case, there're actions specified for `SomeActor`, meaning only these actions will be *papified*. You can specify as many actions as needed.

In this case, **one event set will be used per action**, meaning that if you are selecting two actions, you'll have two event sets in that actor. It has been decided to do it this way for multi-threaded processes, so that every action can be excecuted simultaneously without producing errors in the PAPI output data. Further testing is still required and it is uncertain to me if this is really necesary.

For `SomeOtherActor`, the case is the same as in example 2.




**You can mix the configurations shown in these examples in any way you like. Whenever you specify actions, separated event sets will be used.**

##Output
Once your *papified* ORCC application has been excetued, a new directory called *papi-output* will be created in the current path. For every actor affected by Papify, a *csv* file will be created with several columns:

1. Actor name
2. Action name
3. Event 1
4. Event 2
5. ...

Being there as many columns as events plus two for actor name and action name.

Example of a piece of an actual output csv file:
```
Actor; Action; PAPI_TOT_INS;PAPI_L1_DCM;PAPI_TOT_CYC;
"PCA";"receive_data_PCA_aligned";"22633";"211";"199601"
"PCA";"receive_data_PCA_aligned";"22410";"564";"20121"
"PCA";"receive_data_PCA";"22632";"576";"31281"
"PCA";"receive_data_PCA_aligned";"22410";"564";"20109"
"PCA";"receive_data_PCA";"22638";"578";"39199"
"PCA";"receive_data_PCA_aligned";"22412";"555";"22463"
"PCA";"receive_data_PCA";"22631";"572";"24577"
"PCA";"receive_data_PCA_aligned";"22411";"556";"22274"
"PCA";"receive_data_PCA_aligned";"22410";"571";"29546"
```

Note that every line means one execution of the corresponding action.

##Data processing
From here on you can use the generated data in whatever way you want, but here are a couple of suggestions:

###1. PapiPlot
PapiPlot provides some basic processing and nice graphs for data generated with Papify. You can find it's repository [here on GitHub](https://github.com/panacotta/papiplot)

Here's an example of a histogram generated with PapiPlot:
![Alt text](https://raw.githubusercontent.com/panacotta/papiplot/master/papiplot/readme/papiplot_overall_.png?raw=true "Optional Title")

###2. Pivot tables with Excel
Excel 2013 comes with great, easy to use features to process data with pivot tables
[example image pending]


##Bugs
Papify is still in early development and needs lots of testing. Please contact me if you find any bugs or have feature requests you'd like to see added.
